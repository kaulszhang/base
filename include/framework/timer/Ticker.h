// Ticker.h

#ifndef _FRAMEWORK_TIMER_TICKER_H_
#define _FRAMEWORK_TIMER_TICKER_H_

#include "framework/timer/TimeCounter.h"

namespace framework
{
    namespace timer
    {

        class Ticker
            : public TimeCounter
        {
        public:
            Ticker(
                boost::posix_time::time_duration const & interval, bool is_from_now = false)
                : interval_(interval)
                , is_from_now_(is_from_now)
            {
                next_time_ = time_start_ + interval_;
            }

            Ticker(
                boost::uint32_t msec, bool is_from_now = false)
                : interval_(Duration::milliseconds(msec))
                , is_from_now_(is_from_now)
            {
                next_time_ = time_start_ + interval_;
            }

        public:
            bool check(
                Duration & duration)
            {
                Time now = Time::now();
                if (now > next_time_) {
                    duration = (now - time_start_);
                    if (is_from_now_) {
                        boost::int64_t times = (now - next_time_) / interval_;
                        next_time_ += (interval_ * times);
                    }
                    next_time_ += interval_;
                    return true;
                } else {
                    return false;
                }
            }

            bool check(
                boost::posix_time::time_duration & duration)
            {
                Duration duration2;
                if (check(duration2)) {
                    duration = duration2.to_posix_duration();
                    return true;
                } else {
                    return false;
                }
            }

            bool check(
                boost::uint64_t & msec)
            {
                Duration duration;
                if (check(duration)) {
                    msec = (boost::uint64_t)duration.total_milliseconds();
                    return true;
                } else {
                    return false;
                }
            }

            bool check()
            {
                Duration duration;
                return check(duration);
            }

        private:
            Time next_time_;
            Duration interval_;
            bool is_from_now_;
        };

    } // namespace timer
} // namespace framework

#endif // _FRAMEWORK_TIMER_TICKER_H_
