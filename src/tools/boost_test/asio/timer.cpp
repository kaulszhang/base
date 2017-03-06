// timer.cpp

#include "tools/boost_test/Common.h"

#include <framework/logger/LoggerFormatRecord.h>
using namespace framework::configure;
using namespace framework::logger;

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
using namespace boost::system;
using namespace boost::asio;

FRAMEWORK_LOGGER_DECLARE_MODULE("TestAsioTimer");

static inline void print_error(char const * title, error_code const & ec)
{
    std::cout << title << ": (" << ec.value() << ") " <<  ec.message() << std::endl;
}

static inline void print_error(char const * title, system_error const & err)
{
    std::cout << title << ": (" << err.code().value() << ") " << err.what() << std::endl;
}

void on_timer(boost::system::error_code const & ec)
{
	if (!ec)
            std::cout << "on timer ok" << std::endl;
	else if (ec == error::operation_aborted)
            std::cout << "on timer canceled" << std::endl;
        else
            print_error("timer", ec);
}

void on_timer_cancel(boost::system::error_code const & ec, boost::asio::deadline_timer & t)
{
	t.cancel();
    std::cout << "on timer cancel ok" << std::endl;
}

static void test_asio_timer()
{
    io_service io_svc;
    boost::asio::deadline_timer timer(io_svc);
    timer.expires_from_now(boost::posix_time::seconds(1));
    error_code ec;
    timer.wait(ec);
    on_timer(ec);
}

static void test_asio_timer_async()
{
    io_service io_svc;
    boost::asio::deadline_timer timer(io_svc);
    timer.expires_from_now(boost::posix_time::seconds(1));
    timer.async_wait(on_timer);
    io_svc.run();
}

static void test_asio_timer_async2()
{
    io_service io_svc;
    boost::asio::deadline_timer timer1(io_svc);
	boost::asio::deadline_timer timer2(io_svc);
    timer1.expires_from_now(boost::posix_time::seconds(1000));
	timer2.expires_from_now(boost::posix_time::seconds(1));
    timer1.async_wait(on_timer);
	timer2.async_wait(bind(on_timer_cancel, _1, boost::ref(timer1)));
    io_svc.run();
}

void test_asio_timer(Config & conf)
{
    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[timer] start"));
        test_asio_timer();
        LOG_F(Logger::kLevelDebug, ("[timer] end"));
    }

    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[timer_async] start"));
        test_asio_timer_async();
        LOG_F(Logger::kLevelDebug, ("[timer_async] end"));
    }

	{
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[timer_async2] start"));
        test_asio_timer_async2();
        LOG_F(Logger::kLevelDebug, ("[timer_async2] end"));
    }
}

static TestRegister tr("asio_timer", test_asio_timer);
