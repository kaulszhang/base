// io_service.cpp

#include "tools/boost_test/Common.h"

#include <framework/logger/LoggerFormatRecord.h>
using namespace framework::configure;
using namespace framework::logger;

#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
using namespace boost::system;
using namespace boost::asio;

typedef unsigned long UInt32;

FRAMEWORK_LOGGER_DECLARE_MODULE("TestAsioIoService");

static void post_method(int i)
{
    std::cout << "post_method: " << i << std::endl;
}

static void test_asio_io_service_post()
{
    io_service io_svc;
    io_svc.post(boost::bind(post_method, 1));
    io_svc.post(boost::bind(post_method, 2));
    io_svc.post(boost::bind(post_method, 3));
    io_svc.run();
}

static void wrap_method(int i)
{
    std::cout << "wrap_method: " << i << std::endl;
}

static void test_asio_io_service_wrap()
{
    io_service io_svc;
    io_svc.wrap(boost::bind(wrap_method, _1))(1);
    io_svc.wrap(boost::bind(wrap_method, _1))(2);
    io_svc.wrap(boost::bind(wrap_method, _1))(3);
    io_svc.run();
}

void test_asio_io_service(Config & conf)
{

    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[post] start"));
        test_asio_io_service_post();
        LOG_F(Logger::kLevelDebug, ("[post] end"));
    }

    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[wrap] start"));
        test_asio_io_service_wrap();
        LOG_F(Logger::kLevelDebug, ("[wrap] end"));
    }
}

static TestRegister tr("asio_io_service", test_asio_io_service);
