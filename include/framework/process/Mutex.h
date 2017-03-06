// Mutex.h

#ifndef _FRAMEWORK_PROCESS_MUTEX_H_
#define _FRAMEWORK_PROCESS_MUTEX_H_

#ifdef BOOST_WINDOWS_API
#  define FRAMEWORK_PROCESS_SYNC_OK
#elif defined(__APPLE__)
#  define FRAMEWORK_PROCESS_SYNC_OK
#else
#endif

#ifdef FRAMEWORK_PROCESS_SYNC_OK
#  include <boost/interprocess/sync/interprocess_mutex.hpp>
#  include <boost/thread/locks.hpp>
#  include <boost/thread.hpp>
#
#else
#  include "framework/process/FileMutex.h"
#endif

namespace framework
{
    namespace process
    {

#ifdef FRAMEWORK_PROCESS_SYNC_OK

        class Mutex
            : public boost::interprocess::interprocess_mutex
        {
        public:
            typedef boost::unique_lock<Mutex> scoped_lock;
        };

#else

        class Mutex 
            : public framework::process::FileMutex
        {
        public:
            typedef boost::unique_lock<Mutex> scoped_lock;
        };
#endif
    } // namespace this_process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_MUTEX_H_
