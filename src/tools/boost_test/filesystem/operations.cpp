// operations.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/filesystem/operations.hpp>
using namespace boost::filesystem;

#include <iostream>

static void test_filesystem_operations(Config & conf)
{
    std::cout << "current_path" << current_path() << std::endl;
    create_directory("a");
    exists("a/b/c");
    remove("a/b/c");
    std::cout << "initial_path" << initial_path() << std::endl;
    std::cout << "initial_path" << initial_path() << std::endl;
}

static TestRegister test("filesystem_operations", test_filesystem_operations);
