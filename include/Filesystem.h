#pragma once

#include "Filemap.h"
#include <PATypes/HashMap.h>
#include <PATypes/Map.h>
#include "SHA256.h"
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <fstream>

namespace LabFS {
class Filesystem : std::enable_shared_from_this<Filesystem> {
    enum NodeType { NODE_FILE, NODE_DIRECTORY };
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
			PATypes::MutableArraySequence<char> contents;
			char current;
			while (input >> current) {
				contents.append(current);
			}
			hash = LabFS_Aux::sha256(dynamic_cast<PATypes::Sequence<char>*>(&contents));
        }
		~File() {
			fs.lock()->deindexByHash(hash);
		}
        size_t getHash() { return hash; }
        Path getPath() { return path; }
        std::string getName() { return name; }
    };
    class Node : std::enable_shared_from_this<Node> {
        std::shared_ptr<File> file;
        std::string directoryName;
        std::shared_ptr<PATypes::HashMap<std::string, std::shared_ptr<Node>>>
            subnodes;
        std::weak_ptr<Filesystem> fs;
        std::weak_ptr<Node> parent;

      public:
        Node(std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>())
            : fs(fs) {}
        Node(File file,
             std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>())
            : fs(fs) {
            std::make_shared<File>(file);
        }
        Node(std::string directoryName,
             std::weak_ptr<Filesystem> fs = std::weak_ptr<Filesystem>(),
             std::weak_ptr<Node> parent = std::weak_ptr<Node>())
            : directoryName(directoryName), fs(fs), parent(parent) {
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
        void AddFile(std::shared_ptr<File> file) { file = file; }
        void AddDirectorySubnode(std::string name) {
            if (file != nullptr) {
                throw std::logic_error("попытка добавления директории файлу");
            }
            subnodes->Add(name,
                          std::make_shared<Node>(name, fs, weak_from_this()));
        }
        std::shared_ptr<PATypes::Sequence<std::shared_ptr<Node>>>
        GetChildren() {
            return subnodes->GetAll();
        }
    };
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
    void addFile(std::string filename, Path path) {
		std::shared_ptr<File> file = std::make_shared<File>(filename, path, weak_from_this());

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