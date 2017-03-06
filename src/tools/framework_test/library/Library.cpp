// Library.cpp

#include "tools/framework_test/Common.h"

#include <framework/library/Library.h>
using namespace framework::system;
using namespace framework::configure;
using namespace framework::library;

#include <iostream>

static void test_library(Config & conf)
{
    std::cout << Library::self().path() << std::endl;
}

static TestRegister test("library", test_library);
