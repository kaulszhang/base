// Statistics.h

#ifndef _FRAMEWORK_NETWORK_STATISTICS_H_
#define _FRAMEWORK_NETWORK_STATISTICS_H_

#include <framework/timer/TimeCounter.h>

namespace framework
{
    namespace network
    {

        struct TimeStatistics
            : public framework::timer::TimeCounter
        {
















        protected:
            // ������ϲ������ı�time_start_
            TimeStatistics & operator=(
                TimeStatistics const & r)
            {
                return *this;
            }
        };

    } // namespace network
} // namespace framework

#endif//_FRAMEWORK_NETWORK_STATISTICS_H_
