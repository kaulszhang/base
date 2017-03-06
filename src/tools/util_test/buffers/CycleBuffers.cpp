// CycleBuffers.cpp

#include "tools/util_test/Common.h"

#include <util/buffers/CycleBuffers.h>
using namespace util::buffers;

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
    dump_buffers(buffers.prepare());
    std::cout << "read_buffers" << std::endl;
    dump_buffers(buffers.data());
    std::cout << std::endl;
}

void test_cycle_buffers(Config & conf)
{
    mutable_buffers_1 buffers(buf, sizeof(buf));
    CycleBuffers<mutable_buffers_1> cb(buffers);

    dump_cycle_buffers(cb);
    cb.commit(2);
    std::cout << "commit 2" << std::endl;
    dump_cycle_buffers(cb);
    cb.consume(1);
    std::cout << "consume 1" << std::endl;
    dump_cycle_buffers(cb);
    cb.consume(1);
    std::cout << "consume 1" << std::endl;
    dump_cycle_buffers(cb);
    cb.commit(2);
    std::cout << "commit 2" << std::endl;
    dump_cycle_buffers(cb);
    cb.commit(2);
    std::cout << "commit 2" << std::endl;
    dump_cycle_buffers(cb);
    cb.commit(1020);
    std::cout << "commit 1020" << std::endl;
    dump_cycle_buffers(cb);
    cb.consume(1);
    std::cout << "consume 1" << std::endl;
    dump_cycle_buffers(cb);
    cb.pubseekoff(1023, std::ios_base::cur, std::ios_base::in);
    std::cout << "pubseekoff +1023" << std::endl;
    dump_cycle_buffers(cb);
}

static TestRegister test("cycle_buffers", test_cycle_buffers);
