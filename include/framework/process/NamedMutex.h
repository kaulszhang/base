// NamedMutex.h

#ifndef _FRAMEWORK_PROCESS_NAME_MUTEX_H_
#define _FRAMEWORK_PROCESS_NAME_MUTEX_H_

#ifdef BOOST_WINDOWS_API
#  define FRAMEWORK_PROCESS_SYNC_OK
#endif

#ifdef FRAMEWORK_PROCESS_SYNC_OK
#  include <boost/interprocess/sync/interprocess_mutex.hpp>
#  include <boost/thread/locks.hpp>
#  include <boost/thread.hpp>
#
#else
#  include <boost/interprocess/shared_memory_object.hpp>
#  include "framework/process/FileMutex.h"
#endif

namespace framework
{
    namespace process
    {
#ifdef FRAMEWORK_PROCESS_SYNC_OK

        class NamedMutex
            : public boost::interprocess::interprocess_mutex
        {
        public:
            NamedMutex(boost::int32_t key)
            {
            }
        };

#else
        class NamedMutex : public framework::process::FileMutex
        {
        public:
            NamedMutex( std::string const & keyname )
                //: NamedFileMutex( keyname )
            {
            }

            NamedMutex( boost::int32_t key )
            {
            }

            ~NamedMutex()
            {
            }
        };
#endif

    } // namespace process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_NAME_MUTEX_H_
