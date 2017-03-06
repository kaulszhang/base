// function.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/function.hpp>
#include <boost/mem_fn.hpp>

static void func(char const * v)
{
    std::cout << "func(" << v << ")" << std::endl;
}

struct Func
{
    typedef void result_type;
    void operator()(char const * v) const
    {
        std::cout << "Func(" << v << ")" << std::endl;
    }
};

struct Object
{
    void mem_fun(char const * v) const
    {
        std::cout << "mem_fun(" << v << ")" << std::endl;
    }
};

static void test_function(Config & conf)
{
    boost::function<void (char const *)> f;
    f = func;
    f("1");
    f = Func();
    f("1");
    boost::function<void (Object const *, char const *)> f2;
    f2 = boost::mem_fn(&Object::mem_fun);
    Object obj;
    f2(&obj, "1");
}

static TestRegister test("function", test_function);
