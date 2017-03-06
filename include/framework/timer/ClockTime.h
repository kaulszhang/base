// ClockTime.h

#ifndef _FRAMEWORK_TIMER_CLOCK_TIME_H_
#define _FRAMEWORK_TIMER_CLOCK_TIME_H_

#include <boost/date_time/posix_time/posix_time_config.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/operators.hpp>

namespace framework
{
    namespace timer
    {

        // µ•Œª ∫¡√Î
        class Time;
        class Duration;
        class ClockTime;

        class Duration
            : private boost::less_than_comparable<
                Duration, 
                boost::equality_comparable<Duration>
            >
        {
        public:
            Duration(
                const boost::posix_time::time_duration & d)
            {
                d_ = d.total_milliseconds();
            }

            Duration()
                : d_(0)
            {
            }

        public:
            static Duration seconds(
                boost::int64_t sec)
            {
                return Duration(sec * 1000);
            }

            static Duration milliseconds(
                boost::int64_t msec)
            {
                return Duration(msec);
            }

            static Duration microseconds(
                boost::int64_t msec)
            {
                return Duration(msec / 1000);
            }

            static Duration minutes(
                boost::int64_t min)
            {
                return Duration(min * 60 * 1000);
            }

            boost::posix_time::time_duration to_posix_duration() const
            {
                return boost::posix_time::milliseconds(d_);
            }

        public:
            boost::int64_t total_seconds() const
            {
                return d_ / 1000;
            }

            boost::int64_t total_milliseconds() const
            {
                return d_;
            }

            boost::int64_t total_microseconds() const
            {
                return d_ * 1000;
            }

        public:
            inline Duration & operator+=(
                Duration const & r)
            {
                d_ += r.d_;
                return *this;
            }

            inline Duration & operator-=(
                Duration const & r)
            {
                d_ -= r.d_;
                return *this;
            }

            inline Duration & operator*=(
                size_t n)
            {
                d_ *= n;
                return *this;
            }

            inline Duration & operator/=(
                size_t n)
            {
                d_ /= n;
                return *this;
            }

            friend inline bool operator<(
                Duration const & l, 
                Duration const & r)
            {
                return l.d_ < r.d_;
            }

            friend inline bool operator==(
                Duration const & l, 
                Duration const & r)
            {
                return l.d_ == r.d_;
            }

        private:
            friend inline Duration operator-(
                Time const & l, 
                Time const & r);

            friend inline Time operator+(
                Time const & l, 
                Duration const & r);

            friend inline Time operator-(
                Time const & l, 
                Duration const & r);

            friend inline Duration operator+(
                Duration const & l, 
                Duration const & r)
            {
                return Duration(l.d_ + r.d_);
            }

            friend inline Duration operator-(
                Duration const & l, 
                Duration const & r)
            {
                return Duration(l.d_ + r.d_);
            }

            friend inline Duration operator*(
                Duration const & l, 
                size_t n)
            {
                return Duration(l.d_ * n);
            }

            friend inline Duration operator/(
                Duration const & l, 
                size_t n)
            {
                return Duration(l.d_ / n);
            }

            friend inline boost::int64_t operator/(
                Duration const & l, 
                Duration const & r)
            {
                return l.d_ / r.d_;
            }

        private:
            friend class Time;

            Duration(
                boost::int64_t d)
                : d_(d)
            {
            }

        private:
            boost::int64_t d_;
        };

        class Time
            : private boost::less_than_comparable<
                Time, 
                boost::equality_comparable<Time>
            >
        {
        public:
            static Time now();

        public:
            Time()
            {
                t_ = now().t_;
            }

        public:
            Time & operator+=(
                Duration const & r)
            {
                t_ += r.d_;
                return *this;
            }

            Time & operator-=(
                Duration const & r)
            {
                t_ -= r.d_;
                return *this;
            }

        public:
            friend inline Duration operator-(
                Time const & l, 
                Time const & r)
            {
                return Duration((boost::int64_t)(l.t_ - r.t_));
            }

            friend inline Time operator+(
                Time const & l, 
                Duration const & r)
            {
                return Time(l.t_ + r.d_);
            }

            friend inline Time operator-(
                Time const & l, 
                Duration const & r)
            {
                return Time(l.t_ - r.d_);
            }

            friend inline bool operator<(
                Time const & l, 
                Time const & r)
            {
                return l.t_ < r.t_;
            }

            friend inline bool operator==(
                Time const & l, 
                Time const & r)
            {
                return l.t_ == r.t_;
            }

        private:
            Time(
                boost::uint64_t t)
                : t_(t)
            {
            }

        private:
            boost::uint64_t t_;
        };

        class ClockTime
        {
        public:
            typedef Time time_type;
            typedef Duration duration_type;
        };

    } // namespace timer
} // namespace framework

#endif // _FRAMEWORK_TIMER_CLOCK_TIME_H_
