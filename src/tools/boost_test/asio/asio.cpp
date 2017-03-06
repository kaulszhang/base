// asio.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/asio/error.hpp>
using namespace boost::system;
using namespace boost::asio::error;

void test_asio(Config & conf)
{
    std::cout << "error_code(operation_aborted).message(): " 
        << error_code(operation_aborted).message() << std::endl;
    start_test("asio_io_service", conf);
    start_test("asio_timer", conf);
    start_test("asio_resolver", conf);
    start_test("asio_udp", conf);
    start_test("asio_tcp", conf);
}

static TestRegister tr("asio", test_asio);
