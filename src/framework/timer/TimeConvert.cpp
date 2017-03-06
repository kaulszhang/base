// TimeConvert.cpp

#include "framework/Framework.h"
#include "framework/timer/TimeConvert.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/conversion.hpp>

//#if (defined(__ANDROID__)) && (!defined(__SONY_ANDROID_TV__))
//#include <time64.h>
//#endif

namespace framework
{
    namespace timer
    {
        time_t time_gm( struct tm * tm )
        {
            boost::posix_time::ptime start(boost::gregorian::date(1970,1,1));
            boost::posix_time::ptime pt = boost::posix_time::ptime_from_tm(*tm);
            time_t t = (time_t)(pt - start).total_seconds();
            return t;

//#if (defined( __FreeBSD__ )) || (defined( __MACH__ ))
//            return timegm( tm );
//#elif (defined(__ANDROID__)) && (!defined(__SONY_ANDROID_TV__))
//            return ( time_t )( timegm64( tm ) );
//#else
//            return ( mktime( tm ) - ( time_t )timezone );
//#endif
        }
    } // namespace timer
} // namespace framework
