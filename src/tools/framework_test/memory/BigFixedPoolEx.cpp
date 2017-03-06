// BigFixedPoolEx.cpp

#include "tools/framework_test/Common.h"

#include <framework/memory/BigFixedPoolEx.h>
#include <framework/memory/PrivateMemory.h>
using namespace framework::configure;
using namespace framework::memory;

static void test_big_fixed_pool_ex(Config & conf)
{
    PrivateMemory mem;
    BigFixedPoolEx pool(mem);
    std::vector<void *> vec;
    for (size_t i = 0; i < 1024; ++i) {
        vec.push_back(pool.alloc(rand() % 1024));
    }
    for (size_t i = 0; i < 1023; ++i) {
        size_t j = rand() % (1024 - i);
        pool.free(vec[j]);
        vec[j] = vec[1024 - i - 1];
    }
}

static TestRegister test("big_fixed_pool_ex", test_big_fixed_pool_ex);
