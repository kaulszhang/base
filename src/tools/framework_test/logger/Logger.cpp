// Logger.cpp

#include "tools/framework_test/Common.h"

#include <framework/configure/Config.h>
#include <framework/logger/LoggerListRecord.h>
#include <framework/logger/LoggerFormatRecord.h>
#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::configure;
using namespace framework::logger;

#include <boost/preprocessor/cat.hpp>

std::vector<int> func()
{
    return std::vector<int>(2, 1);
}

#define  STRINGIFY(x) #x 
#define  TOSTRING(x) STRINGIFY(x) 

FRAMEWORK_LOGGER_DECLARE_MODULE("TestLogger");

static void test_logger(Config & conf)
{
    int i = 1;
    float f = 2.0;
    std::string s = "str";
    char m[] = "str";

    //glog.log_r(logger_param_record(0, 0, "TestLog", LoggerParamsBeg() << LOG_NVP(i) << LOG_NVP(f) << LoggerParamsEnd()));
    LOG_L(0, (i)(f)(s)(func()));

    LOG_F(0, "(i=%i,f=%f,s=%s)" % i % f % s.c_str());

    LOG_S(0, "(i=" << i << ",f=" << f << ",s=" << s << ")");
}

static TestRegister test("logger", test_logger);
