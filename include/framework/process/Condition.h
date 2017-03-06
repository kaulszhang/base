// Condition.h

#ifndef _FRAMEWORK_PROCESS_CONDITION_H_
#define _FRAMEWORK_PROCESS_CONDITION_H_

#include "framework/process/Mutex.h"
#include "framework/timer/ClockTime.h"

#ifdef BOOST_WINDOWS_API
#  define FRAMEWORK_PROCESS_SYNC_OK
#endif

#ifndef FRAMEWORK_PROCESS_SYNC_OK
#  include <boost/interprocess/sync/interprocess_condition.hpp>
#else
#  include "framework/memory/SharedMemoryPointer.h"
#  include "framework/process/detail/GlobalSemaphore.h"

#  include <unistd.h>
#  include <sys/stat.h>
#  include <sys/ipc.h>
#  include <fcntl.h>
#endif

namespace framework
{
    namespace process
    {

#ifdef FRAMEWORK_PROCESS_SYNC_OK

        class Condition
            : boost::interprocess::interprocess_condition
        {
        };

#else

        class Condition
        {
        public:
            Condition()
                : value_(0)
            {
            }

            Condition(
                Condition const & r)
                : value_(0)
            {
            }

        public:
            bool wait(
                Mutex & m)
            {
                global_lock();
                int value = value_ + num_++;
                global_unlock();
                m.unlock();
                while ((value - value_) >= 0) {
                    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
                }
                m.lock();
                return true;
            }

            bool timed_wait(
                Mutex & m, 
                ClockTime::time_type const & wait_util)
            {
                global_lock();
                int value = value_ + num_++;
                global_unlock();
                m.unlock();
                bool time_out = false;
                while ((value - value_) >= 0) {
                    if (ClockTime::now() >= wait_util) {
                        global_lock();
                        if (((value - value_) >= 0)) {
                            time_out = true;
                            --num_;
                        }
                        global_unlock();
                        break;
                    }
                    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
                }
                m.lock();
                return time_out;
            }

            template<
                typename TimeDuration
            >
            inline bool timed_wait(
                TimeDuration const & rel_time)
            {
                return timed_wait(ClockTime::now() + rel_time);
            }

            void notify_one()
            {
                global_lock();
                ++value_;
                --num_;
                global_unlock();
            }

            void notify_all()
            {
                global_lock();
                value_ += num_;
                num_ = 0;
                global_unlock();
            }

        private:
            mutable int value_;
            mutable int num_;
        };

#endif

    } // namespace this_process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_CONDITION_H_
