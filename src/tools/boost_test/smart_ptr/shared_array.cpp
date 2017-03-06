// shared_array.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/shared_array.hpp>

static void test_shared_array(Config & conf)
{
    int * vec = new int[2];
    boost::shared_array<int> p(vec);
    p[1] = 0;
    p.reset();
}

static TestRegister test("shared_array", test_shared_array);
