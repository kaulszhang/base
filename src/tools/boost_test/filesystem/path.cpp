// path.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/filesystem/path.hpp>
using namespace boost::filesystem;

#include <iostream>

static void test_filesystem_path(Config & conf)
{
    path ph("/a/b/c/d");
    std::cout << ph << std::endl;
    ph /= "append";
    std::cout << "[append]: " << ph << std::endl;
    std::cout << "file_string: " << ph.string() << std::endl;
    ph.remove_leaf();
    std::cout << "[remove_leaf]: " << ph << std::endl;
    ph.remove_filename();
    std::cout << "[remove_filename]: " << ph << std::endl;
    std::cout << "native_file_string: " << ph.native() << std::endl;
   
    if (ph.has_root_name())
        std::cout << "root_name: " << ph.root_name() << std::endl;
    if (ph.has_root_directory())
        std::cout << "root_directory: " << ph.root_directory() << std::endl;
    if (ph.has_root_path())
        std::cout << "root_path: " << ph.root_path() << std::endl;
    if (ph.has_parent_path())
        std::cout << "parent_path: " << ph.parent_path() << std::endl;
    if (ph.has_relative_path())
        std::cout << "relative_path: " << ph.relative_path() << std::endl;
    std::cout << "branch_path: " << ph.branch_path() << std::endl;
    std::cout << "filename: " << ph.filename() << std::endl;
    std::cout << "leaf: " << ph.leaf() << std::endl;
}

static TestRegister test("filesystem_path", test_filesystem_path);
