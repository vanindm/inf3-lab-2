#pragma once

#include <PATypes/Sequence.h>
#include <string>

namespace LabFS {

const std::string HOMEDIR = "/";
class Path {
    PATypes::LinkedList<std::string> storage;

  public:
    Path(const std::string &path) {
        std::string rpath = path;
        if (rpath[0] != '/') {
            rpath = HOMEDIR + path;
        }
        std::string current_node = "/";
        for (char c : rpath) {
            if (c == '/') {
                if (current_node == "..") {
                    storage.removeAt(storage.getLength() - 1);
                } else {
                    storage.append(current_node);
                }
                current_node = "";
            }
            current_node += c;
        }
    }
    std::string toString() {
        std::string res = "";
        auto iter = storage.getEnumerator();
        while (iter->moveNext()) {
            res += "/";
            res += iter->current();
        }
        return res;
    }
	auto getEnumerator() {
		return storage.getEnumerator();
	}
};

class PathHash {
	public:
    static size_t operator()(Path &path) {
            std::size_t h =
                0; // std::hash<std::string>{}(path.storage.getFirst());
            auto en = path.getEnumerator();
            while (en->moveNext()) {
                h ^= std::hash<std::string>{}(en->current()) + 0x9e3779b9 +
                     (h << 6) + (h >> 2);
            }
            return h;
        }
    };
} // namespace LabFS