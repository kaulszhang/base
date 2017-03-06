// CycleBuffers.cpp

#include "tools/util_test/Common.h"
#include "util/buffers/SubBuffers.h"
using namespace util::buffers;

using namespace framework::configure;

#include <boost/asio/buffer.hpp>
using namespace boost::asio;

char buf[90];

template <typename Buffers>
void dump_buffers(Buffers const & buffers)
{
    std::cout << "dump_buffers:" << std::endl;
    typedef typename Buffers::const_iterator const_iterator;
    for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
        boost::asio::const_buffer buffer(*iter);
        std::cout << buffer_cast<char const *>(buffer) - buf << ":" << buffer_size(buffer) << std::endl;
    }
};

void test_sub_buffers(Config & conf)
{
    std::vector<mutable_buffer> buffers;
    buffers.push_back(mutable_buffer(buf, 40));
    buffers.push_back(mutable_buffer(buf + 40, 20));
    buffers.push_back(mutable_buffer(buf + 60, 30));
    dump_buffers(sub_buffers(buffers, 10));
    dump_buffers(sub_buffers(buffers, 10, 30));
    dump_buffers(sub_buffers(buffers, 10, 50));
    dump_buffers(sub_buffers(buffers, 45));
    dump_buffers(sub_buffers(buffers, 45, 20));
    dump_buffers(sub_buffers(buffers, 45, 50));
}

static TestRegister test("sub_buffers", test_sub_buffers);
