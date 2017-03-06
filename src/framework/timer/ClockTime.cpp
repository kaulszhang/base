// ClockTime.cpp

#include "framework/Framework.h"
#include "framework/timer/ClockTime.h"

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#  include "framework/system/LimitNumber.h"
#elif (defined __MACH__)
#  include <mach/mach_time.h>
#else
#  include <time.h>
#  ifndef CLOCK_MONOTONIC
#    define CLOCK_MONOTONIC 1
#  endif
#include <unistd.h>
#endif

namespace framework
{
    namespace timer
    {
#ifdef __MACH__
	static struct mach_timebase_info get_mach_timebase_info()
        {
            struct mach_timebase_info info;
            ::mach_timebase_info(&info);
            return info;
        }
#endif

        Time Time::now()
        {
#ifdef BOOST_WINDOWS_API
            static framework::system::LimitNumber< 32 > limitNum;
            return Time( limitNum.transfer( ::GetTickCount() ) );
#elif (defined __MACH__)
	static struct mach_timebase_info info = get_mach_timebase_info();
        boost::uint64_t t = mach_absolute_time() / 1000 / 1000;
        if (info.numer != info.denom)
             t = t * info.numer / info.denom;
        return Time(t);
#else
            struct timespec t = { 0 };
            int res = clock_gettime(CLOCK_MONOTONIC, &t);
            (void)res; // gcc warning
            assert(0 == res);
            boost::uint64_t val = ( boost::uint64_t )t.tv_sec * 1000 + t.tv_nsec / 1000 / 1000;
            return Time(val);
#endif
        }

    } // namespace timer
} // namespace framework
