// TimerQueue.h

#ifndef _FRAMEWORK_TIMER_TIMER_QUEUE_H_
#define _FRAMEWORK_TIMER_TIMER_QUEUE_H_

#include "framework/timer/ClockTime.h"

namespace framework
{
    namespace timer
    {

        class Timer;
        class TimerList;
        class QuickTimerList;

        class TimerQueue
        {
        public:
            TimerQueue(
                boost::posix_time::time_duration const & interval,
                bool need_skip = false);

            TimerQueue(
                Duration const & interval,
                bool need_skip = false);

            ~TimerQueue();

        public:
            void schedule_timer(
                Timer & timer);

            void cancel_timer(
                Timer & timer);

            size_t interval_ms() const
            {
                return (size_t)interval_.total_milliseconds();
            }

            size_t alloc_sub_queue(
                size_t interval);

            Time const & next_tick() const
            {
                return next_tick_;
            }

            size_t times() const
            {
                return times_;
            }

        protected:
            size_t skip_to_now();

            void handle_tick();

        private:
            Duration interval_;
            Time next_tick_;
            size_t times_;
            std::vector<TimerList *> timers_;
            bool need_skip_;
        };

    } // namespace timer
} // namespace framework

#endif // _FRAMEWORK_TIMER_TIMER_MANAGER_H_
