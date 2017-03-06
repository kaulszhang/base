// Version.cpp

#include "tools/framework_test/Common.h"

#include <framework/system/Version.h>

using namespace framework::system;
using namespace framework::configure;

static void test_system_version(Config & conf)
{
    std::string version = "1.2.1.299";
    Version::get_version(
        "D:\\trunk\\build\\msvc-90\\trunk\\Debug\\util_test.exe", 
        "framework", version);
    std::cout << version << std::endl;
}

static TestRegister test("system_version", test_system_version);
