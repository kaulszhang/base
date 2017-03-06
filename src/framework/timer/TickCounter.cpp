// TickCounter.cpp

#include "framework/Framework.h"
#include "framework/timer/TickCounter.h"

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#  include "framework/system/LimitNumber.h"
#  ifndef WIN_PHONE
#  pragma comment(lib, "Kernel32.lib")
#  endif  //ifndef WIN_PHONE
#elif (defined __MACH__)
#  include <mach/mach_time.h>
#else
#  include <sys/time.h>
//#  include <time.h>
//#  ifndef CLOCK_MONOTONIC
//#    define CLOCK_MONOTONIC CLOCK_REALTIME
//#  endif
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
        boost::uint64_t TickCounter::tick_count()
        {
#ifdef BOOST_WINDOWS_API
            static framework::system::LimitNumber< 32 > limitNum;
            return limitNum.transfer( ::GetTickCount() );
#elif (defined __MACH__)
            static struct mach_timebase_info info = get_mach_timebase_info();
            boost::uint64_t t = mach_absolute_time() / 1000 / 1000;
            if (info.numer != info.denom)
                t = t * info.numer / info.denom;
            return t;
#else
            struct timespec t = { 0 };
            int res = clock_gettime(CLOCK_MONOTONIC, &t);
            assert(0 == res);
            boost::uint64_t val = ( boost::uint64_t )t.tv_sec * 1000 + t.tv_nsec / 1000 / 1000;
            return val;
#endif
        }

    } // namespace timer
} // namespace framework
