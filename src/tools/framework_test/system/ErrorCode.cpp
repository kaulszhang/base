// ErrorCode.cpp

#include "tools/framework_test/Common.h"

#include <framework/system/ErrorCode.h>

using namespace framework::configure;

static void test_error_code(Config & conf)
{
    boost::system::error_code ec = framework::system::last_system_error();
    std::cout << ec.message() << std::endl;
}

static TestRegister test("error_code", test_error_code);
