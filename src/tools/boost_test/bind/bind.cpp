// bind.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/bind.hpp>

#include <iostream>

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

static void test_bind(Config & conf)
{
    boost::bind(func, "1")();
    boost::bind(func, _1)("1");
    boost::bind(func, _1)("1", 2, 3.0);

    boost::bind(Func(), "1")();
    boost::bind(Func(), _1)("1");
    boost::bind(Func(), _1)("1", 2, 3.0);

    Object obj;
    boost::bind(&Object::mem_fun, obj, "1")();
    boost::bind(&Object::mem_fun, obj, _1)("1");
    boost::bind(&Object::mem_fun, obj, _1)("1", 2, 3.0);
}

static TestRegister test("bind", test_bind);
