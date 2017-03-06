// TimeConvert.h

#ifndef _FRAMEWORK_TIMER_TIME_CONVERT_H_
#define _FRAMEWORK_TIMER_TIME_CONVERT_H_

#include <time.h>

namespace framework
{
    namespace timer
    {
        time_t time_gm( struct tm * tm );
    } // namespace timer
} // namespace framework

#endif // _FRAMEWORK_TIMER_TIME_CONVERT_H_
