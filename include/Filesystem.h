#pragma once

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
class Filesystem : std::enable_shared_from_this<Filesystem> {
    public:
    class File {
        size_t hash;
        Path path;
        std::string name;
        size_t size;
        std::weak_ptr<Filesystem> fs;

      public:
        File(std::string name, Path path,
             std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>())
            : path(path), name(name), fs(fs) {
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
            }
            if (!input.eof() && input.fail()) {
                throw std::length_error("файл не был прочитан");
            }
            PATypes::Sequence<char> *ptr =
                (PATypes::Sequence<char> *)(contents);
            assert(ptr != nullptr);
            hash = LabFS_Aux::sha256(ptr);
            delete contents;
        }

        ~File() {
            if (auto lockedp = fs.lock())
                lockedp->deindexByHash(hash);
        }
        size_t getHash() { return hash; }
        Path getPath() { return path; }
        std::string getName() { return name; }
    };

    enum NodeType { NODE_FILE, NODE_DIRECTORY };
    class Node : std::enable_shared_from_this<Node> {
        std::shared_ptr<File> file;
        std::string directoryName;
        std::shared_ptr<PATypes::HashMap<std::string, std::shared_ptr<Node>>>
            subnodes;
        std::weak_ptr<Filesystem> fs;
        std::weak_ptr<Node> parent;

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
        Node(std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>())
            : fs(fs) {}
        Node(std::shared_ptr<File> file,
             std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>(),
             std::weak_ptr<Node> parent = std::weak_ptr<Node>())
            : file(file), fs(fs) {}
        Node(std::string directoryName,
             std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>(),
             std::weak_ptr<Node> parent = std::weak_ptr<Node>())
            : file(nullptr), directoryName(directoryName), fs(fs),
              parent(parent) {
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
                throw std::logic_error(
                    "попытка получения имени директории файла");
            return directoryName;
        }
        std::shared_ptr<Node> AddFileSubnode(std::shared_ptr<File> file) {
            auto newNode = std::make_shared<Node>(file, fs, weak_from_this());
            subnodes->Add(file->getName(), newNode);
            return newNode;
        }
        std::shared_ptr<Node> AddDirectorySubnode(std::string name) {
            if (file != nullptr) {
                throw std::logic_error("попытка добавления директории файлу");
            }
            auto newNode = std::make_shared<Node>(name, fs, weak_from_this());
            subnodes->Add(name, newNode);
            return newNode;
        }
        std::shared_ptr<PATypes::Sequence<std::shared_ptr<Node>>>
        GetChildren() {
            return subnodes->GetAll();
        }
        std::shared_ptr<File> GetFile() {
            if (file == nullptr) {
                throw std::logic_error("попытка получения файла директории");
            }
            return file;
        }
    };

  private:
    Node root;
    PATypes::Map<size_t, std::shared_ptr<File>> fileStorage;
    PATypes::Map<Path, std::shared_ptr<File>, PathHash> fileByOriginPath;

  public:
    Filesystem() : root(std::string("/"), weak_from_this()) {}
    std::shared_ptr<Node> getRootDirectory() {
        return std::make_shared<Node>(root);
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