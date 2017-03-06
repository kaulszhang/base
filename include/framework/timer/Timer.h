// Timer.h

#ifndef _FRAMEWORK_TIMER_TIMER_H_
#define _FRAMEWORK_TIMER_TIMER_H_

#include "framework/container/List.h"

namespace framework
{
    namespace timer
    {

        class TimerQueue;
        struct TimerAccess;

        class TimerHandler
        {
        protected:
            typedef void (*handle_type)(
                TimerHandler const & rec);

            typedef void (*destroy_type)(
                TimerHandler const & rec);

            TimerHandler(
                handle_type handle, 
                destroy_type destroy)
                : handle_(handle)
                , destroy_(destroy)
            {
            }

            ~TimerHandler()
            {
            }

        public:
            void handle() const
            {
                return handle_(*this);
            }

            void destroy() const
            {
                destroy_(*this);
            }

        private:
            handle_type handle_;
            destroy_type destroy_;
        };

        template <typename Handler>
        class TimerHandlerT
            : public TimerHandler
        {
        public:
            TimerHandlerT(
                Handler const & handler)
                : TimerHandler(&TimerHandlerT::handle, &TimerHandlerT::destroy)
                , handler_(handler)
            {
            }

        private:
            static void handle(
                TimerHandler const & handler)
            {
                TimerHandlerT const & me = static_cast<TimerHandlerT const &>(handler);
                return me.handler_();
            }

            static void destroy(
                TimerHandler const & handler)
            {
                TimerHandlerT const & me = static_cast<TimerHandlerT const &>(handler);
                delete &me;
            }

        private:
            Handler handler_;
        };

        class Timer
            : public framework::container::ListHook<Timer>::type
        {
        public:
            // auto restart (period) timer
            template <typename Handler>
            Timer(
                TimerQueue & queue, 
                size_t interval, 
                Handler const & handler)
                : queue_(&queue)
                , sub_queue_(0)
                , interval_(interval)
                , tick_num_(0)
                , times_(0)
                , handler_(new TimerHandlerT<Handler>(handler))
            {
                assert(interval_ > 0);
                queue_schedule_timer();
            }

            // manual restart
            template <typename Handler>
            Timer(
                TimerQueue & queue, 
                Handler const & handler)
                : queue_(&queue)
                , sub_queue_(0)
                , interval_(0)
                , tick_num_(0)
                , times_(0)
                , handler_(new TimerHandlerT<Handler>(handler))
            {
                queue_schedule_timer();
            }

            ~Timer()
            {
                queue_cancel_timer();
                handler_->destroy();
            }

        public:
            void sub_queue(
                size_t n)
            {
                queue_cancel_timer();
                sub_queue_ = n;
                queue_schedule_timer();
            }

            size_t sub_queue() const
            {
                return sub_queue_;
            }

            void interval(
                size_t n)
            {
                assert(n > 0);
                interval_ = n;
            }

            size_t interval() const
            {
                return interval_;
            }

            size_t times() const
            {
                return times_;
            }

            void start()
            {
                assert(sub_queue_ == 0);
                tick_num_ = interval_;
            }

            void stop()
            {
                tick_num_ = 0;
            }

            void expire_from_now(
                size_t tick_num)
            {
                assert(sub_queue_ == 0);
                assert(tick_num > 0);
                tick_num_ = tick_num;
            }

            size_t expire_from_now() const
            {
                return tick_num_;
            }

            void cancel()
            {
                tick_num_ = 0;
            }

        private:
            friend struct TimerAccess;

            // call by TimerAccess
            void tick()
            {
                if (tick_num_ && --tick_num_ == 0) {
                    ++times_;
                    tick_num_ = interval_;
                    handler_->handle();
                }
            }

            void sub_queue_expire(
                size_t n)
            {
                tick_num_ = n;
            }

            bool sub_queue_tick(
                size_t n)
            {
                if (tick_num_ == n) {
                    ++times_;
                    tick_num_ = 0;
                    handler_->handle();
                    return true;
                } else if (tick_num_ == 0) {
                    return true;
                } else {
                    return false;
                }
            }

            void detach()
            {
                queue_ = NULL;
            }

        protected:
            void queue_schedule_timer();

            void queue_cancel_timer();

            size_t queue_interval_ms() const;

        protected:
            TimerQueue * queue_;
            size_t sub_queue_;
            size_t interval_;
            size_t tick_num_;
            size_t times_;
            TimerHandler * handler_;
        };

        class OnceTimer
            : public Timer
        {
        public:
            template <typename Handler>
            OnceTimer(
                TimerQueue & queue, 
                size_t interval, 
                Handler const & handler)
                : Timer(queue, handler)
                , interval_(interval / queue_interval_ms())
            {
            }

            void start()
            {
                expire_from_now(interval_);
            }

            void stop()
            {
                cancel();
            }

            void interval(
                size_t n)
            {
                interval_ = n / queue_interval_ms();
            }

            size_t interval() const
            {
                return interval_ * queue_interval_ms();
            }

        private:
            size_t interval_;
        };

        class PeriodicTimer
            : public Timer
        {
        public:
            template <typename Handler>
            PeriodicTimer(
                TimerQueue & queue, 
                size_t interval, 
                Handler const & handler)
                : Timer(queue, handler)
            {
                this->interval(interval);
            }

            void interval(
                size_t n)
            {
                Timer::interval(n / queue_interval_ms());
            }

            size_t interval() const
            {
                return Timer::interval() * queue_interval_ms();
            }
        };
    }
}

#endif // _FRAMEWORK_TIMER_TIMER_H_
