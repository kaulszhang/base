// ScaleTransform.cpp

#include "tools/framework_test/Common.h"

#include <framework/system/ScaleTransform.h>
#include <framework/timer/TickCounter.h>
using namespace framework::configure;
using namespace framework::system;
using namespace framework::timer;

static void test_limit_number(Config & conf)
{
    TickCounter c;
    boost::uint64_t in = 5;
    boost::uint64_t out = 3;
    boost::uint64_t n = 0;
    ScaleTransform t(in, out);
    for (boost::uint64_t i = 1; i < 1000000; ++i) {
        n = t.inc(1);
        assert(n == i * out / in);
    }
    std::cout << c.elapsed() << std::endl;
    c.reset();
    for (boost::uint64_t i = 0; i < 1000000; ++i) {
        n = i * out / in;
    }
    std::cout << c.elapsed() << std::endl;
}

static TestRegister test("scale_transform", test_limit_number);
