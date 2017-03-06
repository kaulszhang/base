// time.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/date_time.hpp>

#include <iostream>

static void test_date_time(Config & conf)
{
}

static TestRegister test("date_time", test_date_time);
