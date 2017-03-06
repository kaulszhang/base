// mutex.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
using namespace boost;

void thread_func1(mutex & mtx)
{
    std::cout << "thread_func1: begin" << std::endl;
    mutex::scoped_lock lc(mtx);
    std::cout << "thread_func1: get lock" << std::endl;
    this_thread::sleep(posix_time::seconds(3));
    std::cout << "thread_func1: end" << std::endl;
}

void thread_func2(mutex & mtx)
{
    std::cout << "thread_func2: begin" << std::endl;
    mutex::scoped_lock lc(mtx);
    std::cout << "thread_func2: get lock" << std::endl;
    this_thread::sleep(posix_time::seconds(3));
    std::cout << "thread_func2: end" << std::endl;
}

static void test_mutex(Config & conf)
{
    mutex mtx;
    thread th1(boost::bind(thread_func1, ref(mtx)));
    thread th2(boost::bind(thread_func2, ref(mtx)));
    th1.join();
    th2.join();
}

static TestRegister test("mutex", test_mutex);
