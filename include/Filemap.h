#pragma once

#include <PATypes/Sequence.h>
#include <string>

namespace LabFS {

const std::string HOMEDIR = "/";
class Path {
    PATypes::LinkedList<std::string> storage;

  public:
    Path(const std::string &pathString) {
        std::string rpath = pathString;
        if (rpath[0] != '/') {
            rpath = HOMEDIR + pathString;
        }
        std::string::size_type pos, start = 0;
        while ((pos = rpath.find('/', start)) != std::string::npos) {
            if (pos != 0)
                storage.append(rpath.substr(start, pos - start));
            start = pos + 1;
        }
        storage.append(rpath.substr(start));
    }
    std::string toString() {
        std::string res = "";
        auto iter = storage.getEnumerator();
        while (iter->moveNext()) {
            res += "/";
            res += iter->current();
        } 
        delete iter;
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
            delete en;
            return h;
        }
    };
} // namespace LabFS