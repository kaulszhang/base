// shared_ptr.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

struct Object
{
    void func()
    {
        std::cout << "Object::func()" << std::endl;
    }

    void func2()
    {
    }

    ~Object()
    {
        std::cout << "Object::~Object()" << std::endl;
    }
};

boost::shared_ptr<Object> p;

static void thread_function()
{
    size_t n = 100;
    while (--n) {
        boost::shared_ptr<Object> p1(p);
        p1->func2();
        boost::this_thread::sleep(boost::posix_time::millisec(rand() % 100));
    }
}

static void test_shared_ptr(Config & conf)
{
    p.reset((new Object));
    boost::thread th1(thread_function);
    boost::thread th2(thread_function);
    boost::thread th3(thread_function);
    th1.join();
    th2.join();
    th3.join();
    std::cout << "use_count = " << p.use_count() << std::endl;
    p.reset();
}

static TestRegister test("shared_ptr", test_shared_ptr);
