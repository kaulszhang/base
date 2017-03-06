// Timer.cpp

#include "tools/framework_test/Common.h"

#include <framework/timer/Timer.h>
#include <framework/timer/TickCounter.h>
#include <framework/timer/AsioTimerManager.h>
using namespace framework::system;
using namespace framework::configure;
using namespace framework::timer;

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

class Object
{
public:
    Object(
        TimerQueue & tq, 
        TickCounter & counter, 
        size_t id)
        : t_(tq, 
            boost::bind(&Object::on_timer, this))
        , counter_(counter)
        , id_(id)
    {
        t_.sub_queue(1);
    }

    void on_event()
    {
        if (t_.sub_queue() == 1) {
            t_->sub_queue(2);
        } else {
            t_.sub_queue(1);
        }
    }

private:
    void on_timer()
    {
        std::cout << "Object " << id_ << " on timer " << counter_.elapsed() << std::endl;
        if (t_.sub_queue() == 1) {
            t_->sub_queue(2);
        } else {
            t_.sub_queue(1);
        }
    }

private:
    Timer t_;
    TickCounter const & counter_;
    size_t id_;
};

class TestBed
{
public:
    TestBed(
        Config & conf)
        : manager_(io_svc_, ClockTime::duration_type::seconds(1))
        , t_(manager_, 4, 
            boost::bind(&TestBed::on_timer, this))
    {
        manager_.alloc_sub_queue(10); // 10s
        manager_.alloc_sub_queue(15); // 15s
        objs_.push_back(new Object(manager_, counter_, objs_.size()));
    }

    void run()
    {
        io_svc_.run();
    }

private:
    void on_timer()
    {
        std::cout << "TestBed on timer " << counter_.elapsed() << std::endl;
        objs_.push_back(new Object(manager_, counter_, objs_.size()));
        if (objs_.size() == 5)
            t_.cancel();
    }

private:
    boost::asio::io_service io_svc_;
    AsioTimerManager manager_;
    TickCounter counter_;
    std::vector<Object *> objs_;
    Timer t_;
};

static void test_timer(Config & conf)
{
    TestBed(conf).run();
}

static TestRegister test("timer", test_timer);
