// TimerQueue.cpp

#include "framework/Framework.h"
#include "framework/timer/TimerQueue.h"
#include "framework/timer/Timer.h"
#include "framework/container/List.h"

#include <boost/bind.hpp>

namespace framework
{
    namespace timer
    {

        struct TimerAccess
        {
            static void call_tick(
                Timer & timer)
            {
                timer.tick();
            }

            static void call_sub_queue_expire(
                Timer & timer, 
                size_t n)
            {
                timer.sub_queue_expire(n);
            }

            static bool call_sub_queue_tick(
                Timer & timer, 
                size_t n)
            {
                return timer.sub_queue_tick(n);
            }

            static void call_detach(
                Timer & timer)
            {
                timer.detach();
            }

            static void * get_handler(
                Timer & timer)
            {
                return *(void **)timer.handler_;
            }
        };

        class TimerList
            : public framework::container::List<Timer>
        {
        public:
            TimerList()
            {
                curr_iter_ = end();
            }

            ~TimerList()
            {
                for (curr_iter_ = begin(); curr_iter_ != end(); ) {
                    TimerAccess::call_detach(*curr_iter_++);
                }
            }

        public:
            void tick()
            {
                for (curr_iter_ = begin(); curr_iter_ != end(); ) {
                    Timer & timer = *curr_iter_++;
                    TimerAccess::call_tick(timer);
                }
            }

            void erase(Timer * t)
            {
                if (&*curr_iter_ == t) {
                    ++curr_iter_;
                }
                framework::container::List<Timer>::erase(t);
            }

        protected:
            iterator curr_iter_;
        };

        class QuickTimerList
            : public TimerList
        {
        public:
            QuickTimerList(
                size_t interval)
                : interval_(interval)
                , now_(0)
            {
            }

            void tick()
            {
                if (++now_ == 0)
                    now_ = 1;
                for (curr_iter_ = begin(); curr_iter_ != end(); ) {
                    if (!TimerAccess::call_sub_queue_tick(*curr_iter_++, now_)) {
                        --curr_iter_;
                        break;
                    }
                }
                framework::container::List<Timer>::rotate(curr_iter_);
            }

            size_t interval() const
            {
                return interval_;
            }

            size_t expire() const
            {
                size_t e = now_ + interval_;
                if (e == 0)
                    e = 1;
                return e;
            }

        private:
            size_t interval_;
            size_t now_;
        };

        struct noop_deleter { void operator()(void*) {} };

        TimerQueue::TimerQueue(
            boost::posix_time::time_duration const & interval,
            bool need_skip)
            : interval_(interval)
            , next_tick_(Time::now() + interval_)
            , times_(0)
            , need_skip_(need_skip)
        {
            timers_.push_back(new TimerList);
        }

        TimerQueue::TimerQueue(
            Duration const & interval,
            bool need_skip)
            : interval_(interval)
            , next_tick_(Time::now() + interval_)
            , times_(0)
            , need_skip_(need_skip)
        {
            timers_.push_back(new TimerList);
        }

        TimerQueue::~TimerQueue()
        {
            for (size_t i = 0; i < timers_.size(); ++i)            {
                delete timers_[i];
                timers_[i] = NULL;
            }
        }

        void TimerQueue::schedule_timer(
            Timer & timer)
        {
            size_t sub_queue = timer.sub_queue();
            assert(sub_queue < timers_.size());
            if (sub_queue == 0) {
                timers_[sub_queue]->push_front(&timer);
            } else {
                timers_[sub_queue]->push_back(&timer);
                TimerAccess::call_sub_queue_expire(timer, 
                    ((QuickTimerList *)timers_[sub_queue])->expire());
            }
        }

        void TimerQueue::cancel_timer(
            Timer & timer)
        {
            size_t sub_queue = timer.sub_queue();
            assert(sub_queue < timers_.size());
            timers_[sub_queue]->erase(&timer);
        }

        size_t TimerQueue::alloc_sub_queue(
            size_t interval)
        {
            size_t sub_queue = timers_.size();
            timers_.push_back(new QuickTimerList(interval));
            return sub_queue;
        }

        size_t TimerQueue::skip_to_now()
        {
            Time now = Time::now();
            size_t skip = (size_t)((now - next_tick_) / interval_);
            if (skip)
                next_tick_ += interval_ * skip;
            return skip;
        }

        void TimerQueue::handle_tick()
        {
            ++times_;
            timers_[0]->tick();
            for (size_t i = 1; i < timers_.size(); ++i) {
                ((QuickTimerList *)timers_[i])->tick();
            }

            if (need_skip_ && Time::now() >= next_tick_ + interval_)
            {
                skip_to_now();
            }
            next_tick_ += interval_;
        }

    } // namespace timer
} // namespace framework
