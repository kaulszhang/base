// Parse.cpp

#include "tools/framework_test/Common.h"

#include <framework/string/Parse.h>
#include <framework/string/ParseStl.h>
using namespace framework::configure;
using namespace framework::string;

using namespace boost::system;
typedef std::map<std::string, std::string> string_map;

FRAMEWORK_STRING_CONTAINER_DEFINE(string_map, "", ",", "")
FRAMEWORK_STRING_PAIR_DEFINE(std::string const, std::string, "=")

static void test_parse(Config & conf)
{
    char c = 0;
    parse2("1", c);
    std::cout << c << std::endl;

    unsigned char uc = 0;
    parse2("1", uc);
    std::cout << (int)uc << std::endl;

    int i;
    error_code ec1 = parse2("1", i);
    std::cout << i << std::endl;

    std::map<std::string, std::string> map;
    error_code ec2 = parse2("1=one,2=two,3=three", map);
    //std::cout << map << std::endl;
}

static TestRegister test("parse", test_parse);
