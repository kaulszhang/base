// BytesOrder.cpp

#include "tools/framework_test/Common.h"

#include <framework/system/LimitNumber.h>
using namespace framework::configure;
using namespace framework::system;

unsigned long g_counter = 0xFFFFFFFE;

boost::uint64_t tick_count()
{
#ifdef BOOST_WINDOWS_API
    static LimitNumber< 32 > limitNum;
    return limitNum.transfer( g_counter++ );
#endif
}

static void test_limit_number(Config & conf)
{
    for ( int i = 0; i < 5; ++i )
    {
        printf( "%lld\n", tick_count() );
    }
}

static TestRegister test("limit_number", test_limit_number);
