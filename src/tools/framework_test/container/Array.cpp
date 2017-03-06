// Url.cpp

#include "tools/framework_test/Common.h"

#include <framework/container/Array.h>
using namespace framework::configure;
using namespace framework::container;

static void test_array(Config & conf)
{
    std::vector<int> vec;
    std::vector<int>::iterator iter = vec.begin();
    std::vector<int>::const_iterator citer(vec.begin());
    int v[2];
    Array<const int> arr(v, 2);
    Array<const int>::iterator iter2 = arr.begin();
    Array<const int>::const_iterator citer2(arr.begin());
}

static TestRegister test("array", test_array);
