// TransferBuffers.cpp

#include "tools/util_test/Common.h"
#include "util/stream/detail/transfer_buffers.h"
using namespace util::stream;

using namespace framework::configure;

#include <boost/asio/buffer.hpp>
using namespace boost::asio;

static char buf[1024];

template <typename Buffers>
void dump_buffers(Buffers const & buffers)
{
    typedef typename Buffers::const_iterator const_iterator;
    for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
        boost::asio::const_buffer buffer(*iter);
        std::cout << buffer_cast<char const *>(buffer) - buf << ":" << buffer_size(buffer) << std::endl;
    }
};

template <typename Buffers>
void dump_cycle_buffers(Buffers const & buffers)
{
    std::cout << "write_buffers" << std::endl;
    dump_buffers(buffers.write_buffers());
    std::cout << "read_buffers" << std::endl;
    dump_buffers(buffers.read_buffers());
    std::cout << std::endl;
}

void test_transfer_buffers(Config & conf)
{
    mutable_buffers_1 buffers(buf, sizeof(buf));
    util::stream::detail::transfer_buffers<const_buffer, mutable_buffer, mutable_buffers_1> tb(buffers);

    dump_cycle_buffers(tb);
    tb.commit(2);
    std::cout << "commit 2" << std::endl;
    dump_cycle_buffers(tb);
    tb.consume(1);
    std::cout << "consume 1" << std::endl;
    dump_cycle_buffers(tb);
    tb.consume(1);
    std::cout << "consume 1" << std::endl;
    dump_cycle_buffers(tb);
    tb.commit(2);
    std::cout << "commit 2" << std::endl;
    dump_cycle_buffers(tb);
    tb.commit(2);
    std::cout << "commit 2" << std::endl;
    dump_cycle_buffers(tb);
    tb.commit(1020);
    std::cout << "commit 1020" << std::endl;
    dump_cycle_buffers(tb);
    tb.consume(1);
    std::cout << "consume 1" << std::endl;
    dump_cycle_buffers(tb);
    tb.consume(1023);
    std::cout << "consume 1023" << std::endl;
    dump_cycle_buffers(tb);
}

static TestRegister test("transfer_buffers", test_transfer_buffers);
