// PrivateMemory.cpp

#include "tools/framework_test/Common.h"

#include <framework/memory/PrivateMemory.h>
using namespace framework::configure;
using namespace framework::memory;



static void test_private_memory(Config & conf)
{
    boost::system::error_code ec;
    PrivateMemory mem;
    const size_t size = 15 * 1024 * 1024;
    void * block = mem.alloc_block(size);
    mem.free_block(block, size);
    mem.alloc_with_id( 1, 3 );
    mem.alloc_with_id( 2, 9 );
    mem.alloc_with_id( 3, 33 );
    mem.alloc_with_id( 2, 9 );
    mem.get_by_id( 3 );
    mem.close( ec );
}

static TestRegister test("private_memory", test_private_memory);
