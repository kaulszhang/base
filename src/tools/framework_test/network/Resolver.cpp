// Resolver.cpp

#include <tools/framework_test/Common.h>
#include <framework/logger/LoggerStreamRecord.h>

#include <framework/network/Resolver.h>
using namespace framework::logger;
using namespace framework::configure;

#include <boost/bind.hpp>

using namespace framework::network;

void handle_resolve(
            boost::system::error_code const & ec, 
            ResolverIterator iter) {
    std::cout << "IN ASUNC HANDLE" << std::endl;
    if (ec) {
        std::cout << "ec, value = " << ec.value() << ", message = " << ec.message() << std::endl;
        return;
    }

    ResolverIterator end;
    while (iter != end) {
        std::cout << (*iter).to_string().c_str() << std::endl;
        iter++;
    }
}

void test_network_resolver(Config & conf)
{
    std::string host = "www.baidu.com:90";
    conf.register_module("TestResolver")
        << CONFIG_PARAM_RDWR(host);

    boost::asio::io_service io_svc;
    boost::system::error_code ec;

    {
        NetName dns_name(host);
        Resolver resolver(io_svc);
        ResolverIterator iter = resolver.resolve(dns_name, ec);
        ResolverIterator end;
        while (iter != end) {
            std::cout << (*iter).to_string().c_str() << std::endl;
            iter++;
        }
    }

    {
        std::cout << "TEST 2" << std::endl;
        NetName dns_name(host.c_str());
        Resolver resolver(io_svc);
        ResolverIterator iter = resolver.resolve(dns_name, ec);
        ResolverIterator end;
        while (iter != end) {
            std::cout << (*iter).to_string().c_str() << std::endl;
            iter++;
        }
    }

    {
        std::cout << "TEST 3" << std::endl;
        NetName dns_name(host.c_str());
        Resolver resolver(io_svc);
        resolver.async_resolve(dns_name, handle_resolve);
    }

    io_svc.run();
}

static TestRegister test("network_resolver", test_network_resolver);
