// SmallFixedPool.cpp

#include "tools/framework_test/Common.h"

#include <vector>

#include <framework/memory/MemoryPoolObject.h>
#include <framework/memory/SmallFixedPool.h>
using namespace framework::configure;
using namespace framework::memory;

struct Lock
{
    
};

struct Object
	: MemoryPoolObjectNoThrow<Object, SmallFixedPool>
{
    int i;
};

static void test_small_fixed_pool(Config & conf)
{
    std::vector<Object*> objs;
    for (size_t i = 0; i < 1024; ++i) {
        objs.push_back(new Object);
    }
    for (size_t i = 0; i < 1024; ++i) {
        size_t j = rand() % (1024 - i);
        delete objs[j];
        objs[j] = objs[1024 - i - 1];
    }
}

static TestRegister test("small_fixed_pool", test_small_fixed_pool);
