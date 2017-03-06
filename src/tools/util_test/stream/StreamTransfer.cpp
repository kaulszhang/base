// StreamTransfer.cpp

#include "tools/util_test/Common.h"
#include "util/stream/StreamTransfer.h"
using namespace util::stream;

using namespace framework::configure;

#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
using namespace boost::asio;

static char buf[1024];

void test_stream_transfer(Config & conf)
{
    mutable_buffers_1 buffers(buf, sizeof(buf));

    io_service io_svc;
    ip::tcp::socket s1(io_svc), s2(io_svc);

    transfer(s1, s2, buffers);
}

static TestRegister test("stream_transfer", test_stream_transfer);
