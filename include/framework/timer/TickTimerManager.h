// TickTimerManager.h

#ifndef _FRAMEWORK_TIMER_ASIO_TIMER_MANAGER_H_
#define _FRAMEWORK_TIMER_ASIO_TIMER_MANAGER_H_

#include "framework/timer/TimerQueue.h"

namespace framework
{
    namespace timer
    {

        class TickTimerManager
            : public TimerQueue
        {
        public:
            TickTimerManager(
                boost::posix_time::time_duration const & interval,
                bool need_skip = false);

            TickTimerManager(
                Duration const & interval,
                bool need_skip = false);

            ~TickTimerManager();

        public:
            void reset();

            void tick();

            void tick(
                Time const & now);

        public:
            // 只有在回调函数里调用才有效
            boost::uint32_t elapse() const
            {
                return (boost::uint32_t)elapse_.total_milliseconds();
            }

            boost::uint32_t total_elapse() const
            {
                return (boost::uint32_t)total_elapse_.total_milliseconds();
            }

            boost::uint32_t time_left() const
            {
                return (boost::uint32_t)(next_tick() - last_time_).total_milliseconds();
            }

        private:
            Duration elapse_;
            Duration total_elapse_;
            Time last_time_;
        };

    } // namespace timer
} // namespace framework

#endif // _FRAMEWORK_TIMER_ASIO_TIMER_MANAGER_H_
