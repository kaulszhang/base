// resolver.cpp

#include "tools/boost_test/Common.h"

#include <framework/network/NetName.h>
#include <framework/logger/LoggerFormatRecord.h>
using namespace framework::network;
using namespace framework::configure;
using namespace framework::logger;

#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
using namespace boost;
using namespace boost::system;
using namespace boost::asio;
using namespace boost::asio::ip;

FRAMEWORK_LOGGER_DECLARE_MODULE("TestAsioResolver");

static inline void print_error(char const * title, error_code const & ec)
{
    std::cout << title << ": " << ec.message() << std::endl;
}

static inline void print_error(char const * title, system_error const & err)
{
    std::cout << title << ": " << err.what() << std::endl;
}

tcp::resolver::query make_query(
                               NetName const & addr)
{
    if (addr.family() == NetName::v4) {
        return tcp::resolver::query(tcp::v4(), addr.host(), addr.svc());
    } else if (addr.family() == NetName::v6) {
        return tcp::resolver::query(tcp::v6(), addr.host(), addr.svc());
    } else {
        return tcp::resolver::query(addr.host(), addr.svc());
    }
}

static void handle_tcp_resolver(error_code const & ec, tcp::resolver::iterator i)
{
    tcp::resolver::iterator end;
    while (i != end) {
        std::cout << i->endpoint().address().to_string() << ":" << i->endpoint().port() << std::endl;
        ++i;
    }
}

static void test_asio_tcp_resolver(
                                   NetName const & addr)
{
    try {
        io_service io_svc;
        tcp::resolver resolver(io_svc);
        tcp::resolver::iterator i = 
            resolver.resolve(make_query(addr));
        handle_tcp_resolver(error_code(), i);
    } catch (boost::system::system_error const & err) {
        print_error("tcp_resolver", err);
    }
}

static void test_asio_async_tcp_resolver(
    NetName const & addr)
{
    io_service io_svc;
    tcp::resolver resolver(io_svc);
    resolver.async_resolve(make_query(addr), 
        bind(handle_tcp_resolver, _1, _2));
    io_svc.run();
}

void test_asio_resolver(Config & conf)
{
    NetName addr("www.pptv.com", "http");

    conf.register_module("TestAsioResolver") << CONFIG_PARAM_RDWR(addr);

    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[tcp_resolver] start"));
    }
    test_asio_tcp_resolver(addr);
    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[tcp_resolver] end"));
    }

    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[async_tcp_resolver] start"));
    }
    test_asio_async_tcp_resolver(addr);
    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[async_tcp_resolver] end"));
    }
}

static TestRegister tr("asio_resolver", test_asio_resolver);
