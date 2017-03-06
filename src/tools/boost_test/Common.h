// Common.h

#include <framework/Framework.h>
#include <framework/configure/Config.h>
#include <framework/logger/Logger.h>
#include <framework/logger/LoggerSection.h>

#include <iostream>

typedef void (*FUNC_TEST)(framework::configure::Config & conf);

boost::system::error_code reg_test(
    std::string const & name, 
    FUNC_TEST test);

boost::system::error_code start_test(
    std::string const & name, 
    framework::configure::Config & conf);

boost::system::error_code start_test_all(
    framework::configure::Config & conf);

boost::system::error_code stop_test();

struct TestRegister
{
	TestRegister(
		std::string const & name, 
		FUNC_TEST test)
	{
		boost::system::error_code ec;
		ec = reg_test(name, test);
	}
};
