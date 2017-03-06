// RefenceFromThis.cpp

#include "tools/util_test/Common.h"

using namespace framework::configure;

#include <util/smart_ptr/RefenceFromThis.h>
using namespace util::smart_ptr;

#include <iostream>

struct Base
    : RefenceFromThis<Base>
{
    virtual ~Base()
    {
        std::cout << "~Base" << std::endl;
    };
};

struct Derived
    : Base
{
    void func()
    {
        ref(this);
    }

    virtual ~Derived()
    {
        std::cout << "~Derived" << std::endl;
    };
};

void test_refence_from_this(Config & conf)
{
    boost::intrusive_ptr<Derived> p1(new Derived);
    Base::pointer p2(p1);
    p1->func();
    p1.reset();
    p2.reset();
}

static TestRegister test("refence_from_this", test_refence_from_this);
