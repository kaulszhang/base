// BigFixedPool.cpp

#include "tools/framework_test/Common.h"

#include <framework/memory/MemoryPoolObject.h>
#include <framework/memory/BigFixedPool.h>
using namespace framework::configure;
using namespace framework::memory;

struct Lock
{
};

struct Object
	: MemoryPoolObjectNoThrow<Object, BigFixedPool>
{
    boost::uint8_t nref_;
    boost::uint8_t resv_[2];
    boost::uint8_t buffer_[1024];
    boost::uint8_t resv2_;
};

static void test_big_fixed_pool(Config & conf)
{
    Object::set_pool_capacity(10 * 1024 * 1024);
    std::vector<Object *> objs;
    for (size_t i = 0; i < 10240; ++i) {
        if (objs.empty() || (rand() & 3)) {
            Object * obj = new Object;
            if (obj) {
                objs.push_back(obj);
            } else {
                assert(Object::get_pool().left_object() == 0);
            }
        } else {
            size_t j = rand() % objs.size();
            delete objs[j];
            objs[j] = objs[objs.size() - 1];
            objs.pop_back();
        }
    }
    for (size_t i = 0; i < objs.size(); ++i) {
        delete objs[i];
    }
}

static TestRegister test("big_fixed_pool", test_big_fixed_pool);
