#pragma once

#include "Backref.h"
#include "Filemap.h"
#include "SHA256.h"
#include <PATypes/HashMap.h>
#include <PATypes/Map.h>
#include <cassert>
#include <exception>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace LabFS {
class Filesystem : public std::enable_shared_from_this<Filesystem> {
  public:
    class File {
        size_t hash;
        std::string name;
        Path path;
        size_t size;
        std::weak_ptr<Filesystem> fs;

      public:
        File(std::string name, Path path,
             std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>())
            : name(name), path(path), size(0), fs(fs) {
            std::ifstream input;
            input.open(path.toString(), std::ios::in | std::ios::binary);
            PATypes::MutableArraySequence<char> *contents =
                new PATypes::MutableArraySequence<char>();
            char current;
            while (input >> current) {
                if (input.fail()) {
                    throw std::length_error("файл не был прочитан");
                }
                contents->append(current);
                ++size;
            }
            if (!input.eof() && input.fail()) {
                throw std::length_error(path.toString());
                throw std::length_error("файл не был прочитан");
            }
            input.close();
            PATypes::Sequence<char> *ptr =
                (PATypes::Sequence<char> *)(contents);
            assert(ptr != nullptr);
            hash = LabFS_Aux::pseudoSHA256(ptr);
            delete contents;
        }
        File(const File &file)
            : hash(file.hash), name(file.name), path(file.path),
              size(file.size), fs(file.fs) {}
        File(const File &file, std::weak_ptr<Filesystem> fsptr)
            : hash(file.hash), name(file.name), path(file.path),
              size(file.size) {}

        ~File() {
            if (auto lockedp = fs.lock())
                lockedp->deindexByHash(hash);
        }
        size_t getHash() { return hash; }
        size_t getSize() { return size; }
        Path getPath() { return path; }
        std::string getName() { return name; }
    };

    enum Permissions {
        NOTHING = 0,
        OWNER_READ = 1 << 0,
        OTHERS_READ = 1 << 1,
        OWNER_WRITE = 1 << 2,
        OTHERS_WRITE = 1 << 3,
    };

    class Subject {
        const static unsigned int maxSubjects = 100;
        inline static unsigned int *newId;
        inline static Subject **subjectStorage;
        unsigned int id;
        std::string visibleName;
        Subject() : id(0), visibleName("") {}
        Subject(unsigned int id, const std::string &visibleName)
            : id(id), visibleName(visibleName) {}

      public:
        static Subject *GetSubject(unsigned int id,
                                   const std::string &newName = "") {
            if (newId == nullptr) {
                newId = new unsigned int(0);
                subjectStorage = new Subject *[maxSubjects];
                subjectStorage[*newId] = new Subject(*newId, "root");
                ++(*newId);
                subjectStorage[*newId] = new Subject(*newId, "nobody");
                ++(*newId);
            }
            if (id <= *newId) {
                return subjectStorage[id];
            } else {
                return nullptr;
            }
        }
        static Subject *NewSubject(const std::string &newName) {
            if (newId == nullptr) {
                newId = new unsigned int(0);
                subjectStorage = new Subject *[maxSubjects];
                subjectStorage[*newId] = new Subject(*newId, "root");
                ++(*newId);
                subjectStorage[*newId] = new Subject(*newId, "nobody");
                ++(*newId);
            }
            if (*newId == maxSubjects)
                throw std::overflow_error(
                    "Максимальное количество пользователей достигнуто");
            unsigned int oldId = *newId;
            subjectStorage[*newId] = new Subject(*newId, newName);
            ++(*newId);
            return subjectStorage[oldId];
        }
        bool IsRoot() { return id == 0; }
    };

    enum NodeType { NODE_FILE, NODE_DIRECTORY };
    class Node : public std::enable_shared_from_this<Node>,
                 LabFS_Aux::IReferenceable {
        std::shared_ptr<File> file;
        std::string directoryName;
        std::shared_ptr<PATypes::HashMap<std::string, std::shared_ptr<Node>>>
            subnodes;
        std::weak_ptr<Filesystem> fs;
        std::weak_ptr<Node> parent;
        Subject *owner;
        Permissions permissions;
        void *back_ref;

        void changeFS(std::shared_ptr<Filesystem> fsptr) {
            if (GetType() == NODE_FILE) {
                auto lockedCurrentFS = fs.lock();
                lockedCurrentFS->deindex(file);
            }
        }

      public:
        ~Node() {
            if (GetType() == NODE_DIRECTORY) {
                // auto subnodes = this->subnodes->GetAllPairs();
                // auto subnodesEnumerator = subnodes->getEnumerator();
                // while (subnodesEnumerator->moveNext()) {
                //     this->subnodes->Delete(subnodesEnumerator->current().getFirst());
                // }
            } else {
                if (!fs.expired()) {
                    (*fs.lock()).deindex(file);
                }
            }
        }

        bool CanRead(Subject *subject) {
            if (subject->IsRoot())
                return true;
            if (subject != owner) {
                return permissions & OTHERS_READ;
            } else {
                return permissions & OWNER_READ;
            }
        }

        bool CanWrite(Subject *subject) {
            if (subject->IsRoot())
                return true;
            if (subject != owner) {
                return permissions & OTHERS_WRITE;
            } else {
                return permissions & OWNER_WRITE;
            }
        }

        bool Owns(Subject *subject) {
            return subject == owner || subject->IsRoot();
        }

        void SetPermissions(Subject *subject, Permissions permissions) {
            if (Owns(subject))
                this->permissions = permissions;
            else
                throw std::runtime_error(
                    "Субъект не является владельцем и права менять не может");
        }

        Node(std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>(),
             Subject *owner = Subject::GetSubject(1),
             Permissions permissions = (Permissions)(OWNER_READ | OWNER_WRITE))
            : fs(fs), owner(owner), permissions(permissions) {}

        Node(std::shared_ptr<File> file,
             std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>(),
             std::weak_ptr<Node> parent = std::weak_ptr<Node>(),
             Subject *owner = Subject::GetSubject(1),
             Permissions permissions = (Permissions)(OWNER_READ | OWNER_WRITE))
            : file(file), fs(fs), parent(parent), owner(owner),
              permissions(permissions) {
            auto parentptr = parent.lock();
            if (parentptr != nullptr && !parentptr->CanWrite(owner))
                std::runtime_error(
                    "Субъект не может записывать файлы в данную директорию");
        }

        Node(std::string directoryName,
             std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>(),
             std::weak_ptr<Node> parent = std::weak_ptr<Node>(),
             Subject *owner = Subject::GetSubject(1),
             Permissions permissions = (Permissions)(OWNER_READ | OWNER_WRITE))
            : file(nullptr), directoryName(directoryName), fs(fs),
              parent(parent), owner(owner), permissions(permissions) {
            auto parentptr = parent.lock();
            if (parentptr != nullptr && !parentptr->CanWrite(owner))
                std::runtime_error(
                    "Субъект не может записывать файлы в данную директорию");

            subnodes = std::make_shared<
                PATypes::HashMap<std::string, std::shared_ptr<Node>>>();
        }

        NodeType GetType() {
            return (file == nullptr ? NODE_DIRECTORY : NODE_FILE);
        }

        std::shared_ptr<Node> GetSubnode(std::string name) {
            if (GetType() == NODE_FILE)
                throw std::logic_error("попытка получения директории файла");
            return subnodes->Get(name);
        }

        std::string GetName() {
            if (file != nullptr)
                return file->getName();
            return directoryName;
        }

        std::shared_ptr<Node>
        AddFileSubnode(std::shared_ptr<File> file,
                       Subject *owner = Subject::GetSubject(1)) {
            auto parentptr = parent.lock();
            if (parentptr != nullptr && !parentptr->CanWrite(owner))
                std::runtime_error(
                    "Субъект не может записывать файлы в данную директорию");

            auto newNode = std::make_shared<Node>(file, fs, weak_from_this());
            subnodes->Add(file->getName(), newNode);
            return newNode;
        }

        std::shared_ptr<Node>
        AddDirectorySubnode(std::string name,
                            Subject *owner = Subject::GetSubject(1)) {
            auto parentptr = parent.lock();
            if (parentptr != nullptr && !parentptr->CanWrite(owner))
                std::runtime_error("Субъект не может создавать директории в "
                                   "данной директории");

            if (file != nullptr) {
                throw std::logic_error("Попытка добавления директории файлу");
            }
            auto newNode = std::make_shared<Node>(name, fs, weak_from_this());
            subnodes->Add(name, newNode);
            return newNode;
        }

        std::shared_ptr<PATypes::Sequence<std::shared_ptr<Node>>>
        GetChildren() {
            return subnodes->GetAll();
        }

        std::shared_ptr<File>
        GetFile(Subject *subject = Subject::GetSubject(1)) {
            auto parentptr = parent.lock();
            if (parentptr != nullptr && !parentptr->CanRead(subject))
                std::runtime_error("Субъекту не разрешено читать");

            if (file == nullptr) {
                throw std::logic_error(
                    "Попытка получения файла узла-директории");
            }
            return file;
        }

        void move(std::shared_ptr<Node> target,
                  Subject *subject = Subject::GetSubject(1)) {
            auto parentptr = parent.lock();
            if (parentptr != nullptr && !parentptr->CanWrite(subject))
                std::runtime_error("Субъекту не разрешено записывать");

            auto fsptr = fs.lock();
            auto targetfsptr = target->fs.lock();
            if (fsptr == targetfsptr) {
                if (GetType() == NODE_DIRECTORY) {
                    target->subnodes->Add(directoryName, shared_from_this());
                    parentptr->subnodes->Delete(directoryName);
                    parent = target;
                } else {
                    target->subnodes->Add(file->getName(), shared_from_this());
                    if (parentptr != nullptr)
                        parentptr->subnodes->Delete(file->getName());
                    parent = target;
                }
            }
        }
        void SetReference(void *reference) { this->back_ref = reference; }
        void *GetReference() { return this->back_ref; }
    };

  private:
    std::shared_ptr<Node> root;
    PATypes::Map<size_t, std::shared_ptr<File>> fileStorage;
    PATypes::Map<Path, std::shared_ptr<File>, PathHash> fileByOriginPath;

  public:
    Filesystem() {
        root = std::make_shared<Node>(
            std::string("/"), weak_from_this(), std::weak_ptr<Node>(),
            Subject::GetSubject(0),
            (Permissions) (OTHERS_READ | OTHERS_WRITE | OWNER_READ | OWNER_WRITE));
    }
    std::shared_ptr<Node> getRootDirectory() { return root; }
    std::shared_ptr<Node> getNodeByPath(Path &path) {
        if (path.toString() == "" || path.toString() == "/") {
            return getRootDirectory();
        }
        auto pathEnumerator = path.getEnumerator();
        std::shared_ptr<Node> currentNode = getRootDirectory();
        while (pathEnumerator->moveNext()) {
            currentNode = currentNode->GetSubnode(pathEnumerator->current());
        }
        delete pathEnumerator;
        return currentNode;
    }
    std::shared_ptr<File> getFileByHash(size_t hash) {
        return fileStorage.Get(hash);
    }
    std::shared_ptr<File> addFile(std::string filename, Path path) {
        std::shared_ptr<File> file =
            std::make_shared<File>(filename, path, weak_from_this());
        fileStorage.Add(file->getHash(), file);
        fileByOriginPath.Add(path, file);
        return file;
    }

    void deindexByHash(size_t hash) {
        fileStorage.Delete(hash);
        fileByOriginPath.Delete(fileStorage.Get(hash)->getPath());
    }

    void deindex(std::shared_ptr<File> file) {
        fileStorage.Delete(file->getHash());
        fileByOriginPath.Delete(file->getPath());
    }
};
} // namespace LabFS