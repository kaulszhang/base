// SharedMemory.cpp

#include "tools/framework_test/Common.h"
#include <framework/memory/SharedMemory.h>
#include <framework/system/ErrorCode.h>
using namespace framework::configure;
using namespace framework::logger;
using namespace framework::memory;
using namespace framework::system;

#include <iostream>

struct Head
{
    int * p;
    int * q;
};

#define INSTANCE_ID 5

void test_shared_memory(Config & conf)
{
    bool master = true;
    conf.register_module("TestSharedMemory")
        << CONFIG_PARAM_RDONLY(master);
    boost::system::error_code ec;
	SharedMemory shm;
    if (master) {
        shm.open(INSTANCE_ID, 0, 
            SharedMemory::create | SharedMemory::exclusive, ec);
        std::cout << "create shm: " << ec.message() << std::endl;
    } else {
        shm.open(INSTANCE_ID, 0, 0, ec);
        std::cout << "open shm: " << ec.message() << std::endl;
    }
    Head * h = (Head *)shm.alloc_with_id(0, sizeof(Head));
    Head * h1 = (Head *)shm.alloc_with_id(1, sizeof(Head));
    Head * h2 = (Head *)shm.alloc_with_id(2, sizeof(Head));
    Head * h4 = (Head *)shm.alloc_with_id(4, sizeof(Head));
    Head * h3 = (Head *)shm.alloc_with_id(3, sizeof(Head));
    void * b1 = shm.alloc_block(20);
    void * b2 = shm.alloc_block(20);
    void * b3 = shm.alloc_block(20);
}

static TestRegister test("shared_memory", test_shared_memory);
