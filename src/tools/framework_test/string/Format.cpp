// Parse.cpp

#include "tools/framework_test/Common.h"

#include <framework/string/Format.h>
#include <framework/string/FormatStl.h>
using namespace framework::system;
using namespace framework::configure;
using namespace framework::string;

static void test_format(Config & conf)
{
    char c = 'f';

    unsigned char uc = 'f';

    std::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    std::map<std::string, std::string> map;
    map["1"] = "one";
    map["2"] = "two";
    map["3"] = "three";

    std::string str_char = format(c);
    std::cout << str_char << std::endl;

    std::string str_uchar = format(uc);
    std::cout << str_uchar << std::endl;

    std::string str_vec = format(vec);
    std::cout << str_vec << std::endl;

    std::string str_map = format(map);
    std::cout << str_map << std::endl;
}

static TestRegister test("format", test_format);
