// TcpSocket.cpp

#include "tools/framework_test/Common.h"

#include <framework/network/TcpSocket.h>
using namespace framework::system;
using namespace framework::configure;
using namespace framework::network;

#include <boost/thread/thread.hpp>

void on_async_conect(
                     boost::system::error_code const & ec)
{
    std::cout << "async_connect returned, ec = " << ec.message() << std::endl;
}
void test_async_connector()
{
    NetName addr("1.2.3.4:80");
    boost::system::error_code ec;
    boost::asio::io_service ios;

    TcpSocket socket(ios);
    socket.set_time_out(3, ec);
    socket.async_connect(addr, boost::bind(on_async_conect, _1));
    ios.run();
}

static void test_network_tcp_socket(Config & conf)
{
    test_async_connector();
    //NetName addr("192.168.1.109", 1802);
    //std::string name;

    //conf.register_module("TestTcpSocket")
    //    << CONFIG_PARAM_RDWR(addr)
    //    << CONFIG_PARAM_RDWR(name);

    //boost::system::error_code ec;
    //boost::asio::io_service io_svc;
    //TcpSocket socket(io_svc);
    //socket.set_non_block(true, ec);
    //socket.connect(addr, ec);
    //while (ec == boost::asio::error::would_block) {
    //    std::cout << "connect:" << ec.message() << std::endl;
    //    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    //    socket.connect(addr, ec);
    //};
    //std::cout << "connect:" << ec.message() << std::endl;
}

static TestRegister test("network_tcp_socket", test_network_tcp_socket);
