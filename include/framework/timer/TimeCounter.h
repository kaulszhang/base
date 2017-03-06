// TimeCounter.h

#ifndef _FRAMEWORK_TIMER_TIME_COUNTER_H_
#define _FRAMEWORK_TIMER_TIME_COUNTER_H_

#include "framework/timer/ClockTime.h"

namespace framework
{
    namespace timer
    {

        struct TimeCounter
        {
        public:
            TimeCounter()
            {
            }

        public:
            void reset()
            {
                time_start_ = Time::now();
            }

            boost::uint32_t elapse() const
            {
                return (boost::uint32_t)(Time::now() - time_start_).total_milliseconds();
            }

        public:
            Time time_start_;
        };

    } // namespace timer
} // namespace framework

#endif // _FRAMEWORK_TIMER_TIME_COUNTER_H_
