// TimeTraits.h

#ifndef _FRAMEWORK_TIMER_TIME_TRAITS_H_
#define _FRAMEWORK_TIMER_TIME_TRAITS_H_

#include <framework/timer/ClockTime.h>
using namespace framework::timer;

#include <boost/asio/basic_deadline_timer.hpp>

typedef boost::asio::basic_deadline_timer<framework::timer::ClockTime> clock_timer;

namespace boost {
namespace asio {

/// Time traits specialised for framework::timer::ClockTime.
template <>
struct time_traits<framework::timer::ClockTime>
{
    /// The time type.
    typedef framework::timer::ClockTime::time_type time_type;

    /// The duration type.
    typedef framework::timer::ClockTime::duration_type duration_type;

    /// Get the current time.µ•Œª∫¡√Î
    static time_type now()
    {
        return time_type::now();
    }

    /// Add a duration to a time.
    static time_type add(const time_type& t, const duration_type& d)
    {
        return t + d;
    }

    /// Subtract one time from another.
    static duration_type subtract(const time_type& t1, const time_type& t2)
    {
        return t1 - t2;
    }

    /// Test whether one time is less than another.
    static bool less_than(const time_type& t1, const time_type& t2)
    {
        return t1 < t2;
    }

    /// Convert to POSIX duration type.
    static boost::posix_time::time_duration to_posix_duration(
        const duration_type & d)
    {
        return d.to_posix_duration();
    }
};


} // namespace asio
} // namespace boost

#endif // _FRAMEWORK_TIMER_TIME_TRAITS_H_
