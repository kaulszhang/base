// TickCounter.cpp

#include "tools/framework_test/Common.h"

#include <framework/timer/TickCounter.h>
using namespace framework::system;
using namespace framework::configure;
using namespace framework::timer;

#include <boost/thread/thread.hpp>

static void test_tick_counter(Config & conf)
{
    TickCounter c;
    std::cout << "start ticks: " << TickCounter::tick_count() << std::endl;
    std::cout << "will sleep 1s" << std::endl;
    boost::this_thread::sleep(boost::posix_time::seconds(1));
    std::cout << "end ticks: " << TickCounter::tick_count() << std::endl;
    std::cout << "elapsed ticks: " << c.elapsed() << std::endl;
}

static TestRegister test("tick_counter", test_tick_counter);
