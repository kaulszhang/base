// Timer.cpp

#include "framework/Framework.h"
#include "framework/timer/Timer.h"
#include "framework/timer/TimerQueue.h"

namespace framework
{
    namespace timer
    {

        void Timer::queue_schedule_timer()
        {
            queue_->schedule_timer(*this);
        }

        void Timer::queue_cancel_timer()
        {
            if (queue_)
                queue_->cancel_timer(*this);
        }

        size_t Timer::queue_interval_ms() const
        {
            return queue_->interval_ms();
        }

    } // namespace timer
} // namespace framework
