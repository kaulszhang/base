// udp.cpp

#include "tools/boost_test/Common.h"

#include <framework/network/NetName.h>
#include <framework/logger/LoggerFormatRecord.h>
using namespace framework::configure;
using namespace framework::logger;
using namespace framework::network;

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
using namespace boost::system;
using namespace boost::asio;
using namespace boost::asio::ip;

typedef unsigned long UInt32;

FRAMEWORK_LOGGER_DECLARE_MODULE("TestAsioUdp");

static inline void print_error(char const * title, error_code const & ec)
{
    std::cout << title << ": " << ec.message() << std::endl;
}

static inline void print_error(char const * title, system_error const & err)
{
    std::cout << title << ": " << err.what() << std::endl;
}

static void test_asio_udp_client(
                                 NetName const & addr)
{
    std::string input = "test_asio_udp";

    {
        std::cout << "input:" << input << std::endl;
    }

    streambuf buf;
    {
        std::ostream os(&buf);
        os << input;
    }

    try {
        io_service io_svc;
        udp::socket fd(io_svc);
        fd.open(udp::v4());
        fd.connect(udp::endpoint(address::from_string(addr.host()), addr.port()));
        std::cout << "udp_client: connection established" << std::endl;
        size_t len = 0;
        len = fd.send(buf.data());
        std::cout << "udp_client: request sent, len = " << len  << std::endl;
        buf.consume(buf.size());
        len = fd.receive(buf.prepare(1024));
        buf.commit(len);
        std::cout << "udp_client: response received, len = " << len << std::endl;
    } catch (boost::system::system_error const & err) {
        print_error("udp_client", err);
        return;
    }

    std::string output;
    {
        std::istream is(&buf);
        is >> output;
    }

    {
        std::cout << "output:" << output << std::endl;
    }
};

static void test_asio_udp_server(
                                 NetName const & addr)
{
    try {
        io_service io_svc;
        udp::socket fd(io_svc);
        fd.open(udp::v4());
        fd.bind(udp::endpoint(address::from_string(addr.host()), addr.port()));
        streambuf buf;
        udp::endpoint endp;
        size_t len = fd.receive_from(buf.prepare(1024), endp);
        buf.commit(len);
        fd.send_to(buf.data(), endp);
        buf.consume(buf.size());
    } catch (boost::system::system_error const & err) {
        print_error("udp_server", err);
        return;
    }
}


static void test_asio_udp_client_2()
{
    char buff[10241] = {0};
    for (int i = 0; i < 10; i++)
    {
        memset(buff+i*1024, 'A'+i, 1024);
    }
    streambuf buf;
    {
        std::ostream os(&buf);
        os << buff;
    }
    try 
    {
        io_service io_svc;
        udp::socket fd(io_svc);
        fd.open(udp::v4());
        fd.connect(udp::endpoint(address::from_string("127.0.0.1"), 1111));
        std::cout << "udp_client: connection established" << std::endl;
        size_t len = 0;
        len = fd.send(buf.data());
        std::cout << "udp_client: request sent, len = " << len  << std::endl;
        buf.consume(buf.size());
        len = fd.receive(buf.prepare(10240));
        buf.commit(len);
        std::cout << "udp_client: response received, len = " << len << std::endl;
    } 
    catch (boost::system::system_error const & err) 
    {
        print_error("udp_client", err);
        return;
    }

    std::string output;
    {
        std::istream is(&buf);
        is >> output;
    }

    {
        std::cout << "output:" << output << std::endl;
    }
};

static void test_asio_udp_server_2()
{
    try 
    {
        io_service io_svc;
        udp::socket fd(io_svc);
        fd.open(udp::v4());
        fd.bind(udp::endpoint(address::from_string("127.0.0.1"), 1111));
        streambuf buf;
        udp::endpoint endp;
        size_t len = fd.receive_from(buf.prepare(10240), endp);
        buf.commit(len);
        fd.send_to(buf.data(), endp);
        buf.consume(buf.size());
        std::string output;
        {
            std::istream is(&buf);
            is >> output;
        }

        {
            std::cout << "output:" << output << std::endl;
        }
    } 
    catch (boost::system::system_error const & err) 
    {
        print_error("udp_server2", err);
        return;
    }
}



class test_asio_async_udp_client_t
{
public:
    test_asio_async_udp_client_t(
        io_service & io_svc)
        : fd(io_svc)
    {
    }

    void start(
        NetName const & addr)
    {
        std::string input = "test_asio_async_udp";
        std::cout << "input:" << input << std::endl;
        {
            std::ostream os(&buf);
            os << input;
        }
        error_code ec;
        fd.open(udp::v4(), ec);
        if (!ec)
            fd.connect(udp::endpoint(address::from_string(addr.host()), addr.port()), ec);
        if (ec) {
            handle_error(ec);
        } else {
            fd.async_send(buf.data(), 
                boost::bind(&test_asio_async_udp_client_t::handle_send, this, _1, _2));
        }
    }

private:
    void handle_send(
        error_code const & ec, 
        size_t len)
    {
        if (ec) {
            handle_error(ec);
        } else {
            buf.consume(buf.size());
            fd.async_receive(buf.prepare(1024), 
                boost::bind(&test_asio_async_udp_client_t::handle_receive, this, _1, _2));
        }
    }

    void handle_receive(
        error_code const & ec, 
        size_t len)
    {
        if (ec) {
            handle_error(ec);
        } else {
            buf.commit(len);
            std::string output;
            {
                std::istream is(&buf);
                is >> output;
            }
            std::cout << "output:" << output << std::endl;
            delete this;
        }
    }

    void handle_error(
        error_code const & ec)
    {
        print_error("async_udp_client", ec);
        delete this;
    }

private:
    udp::socket fd;
    streambuf buf;
};

static void test_asio_async_udp_client(
                                       io_service & io_svc, 
                                       NetName const & addr)
{
    (new test_asio_async_udp_client_t(io_svc))->start(addr);
};

class test_asio_async_udp_server_t
{
public:
    test_asio_async_udp_server_t(
        io_service & io_svc)
        : fd(io_svc)
    {
    }

    void start(
        NetName const & addr)
    {
        error_code ec;
        fd.open(udp::v4(), ec);
        if (!ec)
            fd.bind(udp::endpoint(address::from_string(addr.host()), addr.port()), ec);
        if (ec) {
            handle_error(ec);
        } else {
            fd.async_receive_from(buf.prepare(1024), endp, 
                boost::bind(&test_asio_async_udp_server_t::handle_receive_from, this, _1, _2));
        }
    }

private:
    void handle_receive_from(
        error_code const & ec, 
        size_t len)
    {
        if (ec) {
            handle_error(ec);
        } else {
            buf.commit(len);
            fd.async_send_to(buf.data(), endp, 
                boost::bind(&test_asio_async_udp_server_t::handle_send_to, this, _1, _2));
        }
    }

    void handle_send_to(
        error_code const & ec, 
        size_t len)
    {
        if (ec) {
            handle_error(ec);
        } else {
            buf.consume(buf.size());
            delete this;
        }
    }

    void handle_error(
        error_code const & ec)
    {
        print_error("async_udp_server", ec);
        delete this;
    }

private:
    udp::socket fd;
    streambuf buf;
    udp::endpoint endp;
};

static void test_asio_async_udp_server(
                                       io_service & io_svc, 
                                       NetName const & addr)
{
    (new test_asio_async_udp_server_t(io_svc))->start(addr);
}

void test_asio_udp(Config & conf)
{
    int mode = 3; // 1 - sync server, 2 - sync client
    // 4 - async server, 8 - async client
    NetName addr("127.0.0.1", 1111);

    conf.register_module("TestAsioUdp") << CONFIG_PARAM_RDWR(mode) << CONFIG_PARAM_RDWR(addr);

    if (mode & 3) {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[udp] start"));
        boost::thread * threads[2] = {NULL};
        if (mode & 1) {
            threads[0] = new boost::thread(boost::bind(test_asio_udp_server, addr));
        }
        if (mode & 2) {
            threads[1] = new boost::thread(boost::bind(test_asio_udp_client, addr));
        }
        if (mode & 1) {
            threads[0]->join();
            delete threads[0];
        }
        if (mode & 2) {
            threads[1]->join();
            delete threads[1];
        }
        LOG_F(Logger::kLevelDebug, ("[udp] end"));
    }

    if (mode & 12) {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[async_udp] start"));
        io_service io_svc;
        if (mode & 4) {
            test_asio_async_udp_server(io_svc, addr);
        }
        if (mode & 8) {
            test_asio_async_udp_client(io_svc, addr);
        }
        io_svc.run();
        LOG_F(Logger::kLevelDebug, ("[async_udp] end"));
    }
}

static TestRegister tr("asio_udp", test_asio_udp);
