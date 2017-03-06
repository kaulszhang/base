// TickTimerManager.cpp

#include "framework/Framework.h"
#include "framework/timer/TickTimerManager.h"
#include "framework/logger/LoggerSection.h"
using namespace framework::logger;

namespace framework
{
    namespace timer
    {

        struct noop_deleter { void operator()(void*) {} };

        TickTimerManager::TickTimerManager(
            boost::posix_time::time_duration const & interval,
            bool need_skip)
            : TimerQueue(interval, need_skip)
        {
        }

        TickTimerManager::TickTimerManager(
            Duration const & interval,
            bool need_skip)
            : TimerQueue(interval, need_skip)
        {
        }

        TickTimerManager::~TickTimerManager()
        {
        }

        void TickTimerManager::reset()
        {
            skip_to_now();
            last_time_ = Time::now();
            total_elapse_ = Duration();
        }

        void TickTimerManager::tick()
        {
            tick(Time::now());
        }

        void TickTimerManager::tick(
            Time const & now)
        {
            if (now >= next_tick()) {
                elapse_ += (now - next_tick());
                handle_tick();
                elapse_ = (next_tick() - now);
            } else {
                total_elapse_ += (now - last_time_);
            }
            last_time_ = now;
        }

    } // namespace timer
} // namespace framework
