// SharedMemoryPointer.cpp

#include "tools/framework_test/Common.h"
#include <framework/memory/SharedMemory.h>
#include <framework/memory/SharedMemoryIdPointer.h>
#include <framework/memory/SharedMemoryPointer.h>
#include <framework/memory/MemoryReference.h>
#include <framework/memory/MemoryPoolObject.h>
#include "framework/memory/SmallFixedPool.h"
#include <framework/container/List.h>
#include <framework/system/ErrorCode.h>
using namespace framework::configure;
using namespace framework::logger;
using namespace framework::memory;
using namespace framework::container;
using namespace framework::system;

#include <boost/ref.hpp>

#define INSTANCE_ID 5

struct Head
    : ListHook<Head, SharedMemoryPointerTraits<Head, INSTANCE_ID> >::type
    , MemoryPoolObjectNoThrow<Head, SmallFixedPool>
{
    Head(int p, int q)
    {
        this->p = p;
        this->q = q;
    }
    int p;
    int q;
    
    static SharedMemory shm;

    static SmallFixedPool init_pool()
    {
        return SmallFixedPool(MemoryReference<SharedMemory>(shm));
    }
};

SharedMemory Head::shm;

inline std::ostream & operator<<(
                                 std::ostream & os, 
                                 Head const & h)
{
    os << h.p << " " << h.q;
    return os;
}

typedef List<Head> list_type;

void test_shared_memory_pointer(Config & conf)
{
    bool master = true;
    conf.register_module("TestSharedMemoryPointer")
        << CONFIG_PARAM_RDONLY(master);
    boost::system::error_code ec;
    if (master) {
        std::cout << "create shm: " << ec.message() << std::endl;
        Head::shm.open(INSTANCE_ID, 0, 
            SharedMemory::create | SharedMemory::exclusive, ec);
        Head * h0 = new Head(0, 10);
        Head * h1 = new Head(1, 11);
        Head * h2 = new Head(2, 12);
        list_type & list = *(new (Head::shm, 3) list_type);
        list.push_back(h0);
        list.push_back(h1);
        list.push_back(h2);
        if (master) {
            while (Head * h = list.first()) {
                list.erase(h);
                delete h;
            }
        }
    } else {
        std::cout << "open shm: " << ec.message() << std::endl;
        Head::shm.open(INSTANCE_ID, 1, 0, ec);
        list_type & list = *SharedMemoryIdPointer<list_type, INSTANCE_ID>(3);
        std::cout << "list.size() = " << list.size() << std::endl;
        for (list_type::const_iterator i = list.begin(); i != list.end(); ++i) {
            std::cout << "  " << *i << std::endl;
        }
    }
}

static TestRegister test("shared_memory_pointer", test_shared_memory_pointer);
