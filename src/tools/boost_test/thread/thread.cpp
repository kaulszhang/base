// thread.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
using namespace boost;

void thread_func( int i, float f )
{
    std::cout << i << "\t" << f << std::endl;
    std::cout << "thread_func: begin" << std::endl;
    this_thread::sleep(posix_time::seconds(3));
    std::cout << "thread_func: end" << std::endl;
}

typedef void ( *fun_call )( int, float );

class COther
{
public:
    COther(){}
    void thread_fun( int i, float f, fun_call func )
    {
         func( i, f );
    }
};

class CThread
{
public:
    CThread()
    {
        // 绑定自身类中的成员函数
        th_ = new boost::thread( boost::bind( &CThread::thread_func, this, 2, 4 ) );
        th_->join();
    }

    ~CThread()
    {
        delete th_;
    }

    void other_class_thread_fun( int i, float f )
    {
        if ( th_ ) delete th_;

        COther * other = new COther();

        // 绑定其它类中的成员函数
        th_ = new boost::thread( boost::bind( &COther::thread_fun, other, i, f, ::thread_func ) );
        th_->join();
        delete other;
    }

    void thread_func( int i, float f )
    {
        std::cout << i << "\t" << f << std::endl;
    }

private:
    boost::thread * th_;
};

static void test_thread(Config & conf)
{
    // 绑定全局成员函数
    thread th(boost::bind(thread_func, 5, 2.2f ));
    th.join();

    CThread ct;
    ct.other_class_thread_fun( 78, 4.67f );
}

static TestRegister test("thread", test_thread);
