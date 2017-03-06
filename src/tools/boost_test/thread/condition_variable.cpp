// condition_variable.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/bind.hpp>
using namespace boost;

void thread_func1(mutex & mtx, condition_variable & cv)
{
    std::cout << "thread_func1: begin" << std::endl;
    this_thread::sleep(posix_time::seconds(3));
    mutex::scoped_lock lc(mtx);
    cv.notify_one();
    std::cout << "thread_func1: notify_one" << std::endl;
    std::cout << "thread_func1: end" << std::endl;
}

void thread_func2(mutex & mtx, condition_variable & cv)
{
    std::cout << "thread_func2: begin" << std::endl;
    mutex::scoped_lock lc(mtx);
    cv.wait(lc);
    std::cout << "thread_func2: get lock" << std::endl;
    std::cout << "thread_func2: end" << std::endl;
}

static void test_condition_variable(Config & conf)
{
    mutex mtx;
    condition_variable cv;
    thread th1(boost::bind(thread_func1, ref(mtx), ref(cv)));
    thread th2(boost::bind(thread_func2, ref(mtx), ref(cv)));
    th1.join();
    th2.join();
}

static TestRegister test("condition_variable", test_condition_variable);
