// Algorithm.cpp

#include "tools/framework_test/Common.h"

#include <framework/string/Algorithm.h>
using namespace framework::string;
using namespace framework::configure;

static void test_string_algorithm(Config & conf)
{
    std::string str = "   ";
    trim(str);
    std::string str_test = "abc de";
    string_limit limt( 0, str_test.size() );
    trim( str_test, limt );

    bool eq = strncasecmp( "Abc", "aBC" );
}

static TestRegister test("string_algorithm", test_string_algorithm);
