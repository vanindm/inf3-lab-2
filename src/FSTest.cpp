#include "Filesystem.h"
#include <cassert>
#include <iostream>
#include <string>

int main() {
	LabFS::Filesystem fs;
    auto root = fs.getRootDirectory();
    root->AddDirectorySubnode("test");
    auto children = root->GetChildren();
    auto iterDirectories = children->getEnumerator();
    while (iterDirectories->moveNext()) {
            assert(iterDirectories->current()->GetName() ==
    std::string("test"));
    }
    delete iterDirectories;
    assert(root->GetSubnode("test")->GetName() == "test");
    root->AddFileSubnode(fs.addFile("etcpasswd", LabFS::Path("/etc/passwd")));
    return 0;
}