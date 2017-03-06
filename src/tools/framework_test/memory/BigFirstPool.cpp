// BigFirstPool.cpp

#include "tools/framework_test/Common.h"

#include <framework/memory/BigFirstPool.h>
#include <framework/memory/PrivateMemory.h>
using namespace framework::configure;
using namespace framework::memory;

#include <boost/thread/mutex.hpp>
using namespace boost;

BigFirstPool & get_pool()
{
    PrivateMemory mem;
    static BigFirstPool pool(mem);
    return pool;
}

static mutex & get_mutex()
{
    static mutex mtx;
    return mtx;
}

void * operator new (size_t size)
{
    mutex::scoped_lock lock(get_mutex());
    return get_pool().alloc(size);
}

void operator delete (void * ptr)
{
    mutex::scoped_lock lock(get_mutex());
    get_pool().free(ptr);
}

void * operator new[] (size_t size)
{
    mutex::scoped_lock lock(get_mutex());
    return get_pool().alloc(size);
}

void operator delete[] (void * ptr)
{
    mutex::scoped_lock lock(get_mutex());
    get_pool().free(ptr);
}

static void test_big_first_pool(Config & conf)
{
    std::vector<char *> vec;
    for (size_t i = 0; i < 1024; ++i) {
        vec.push_back(new char[(rand() % 1024)]);
    }
    get_pool().debug_dump();
    for (size_t i = 0; i < 1023; ++i) {
        size_t j = rand() % (1024 - i);
        delete [] vec[j];
        vec[j] = vec[1024 - i - 1];
    }
}

static TestRegister test("big_first_pool", test_big_first_pool);
