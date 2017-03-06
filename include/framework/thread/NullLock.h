// NullLock.h

#ifndef _FRAMEWORK_THREAD_NULL_LOCK_H_
#define _FRAMEWORK_THREAD_NULL_LOCK_H_

#include <boost/thread/locks.hpp>

#ifdef NDEBUG
#undef FRAMEWORK_THREAD_NULL_LOCK_ASSERT
#endif

#ifdef FRAMEWORK_THREAD_NULL_LOCK_ASSERT
#include <boost/thread/thread.hpp>
#endif

namespace framework
{
	namespace thread
	{

		class NullLock
		{
        public:
#ifdef FRAMEWORK_THREAD_NULL_LOCK_ASSERT
            NullLock()
            {
            }
#endif

			void lock()
			{
#ifdef FRAMEWORK_THREAD_NULL_LOCK_ASSERT
                if (tid_ == boost::thread::id()) {
                    tid_ = boost::this_thread::get_id();
                }
                assert(boost::this_thread::get_id() == tid_);
#endif
			}

            void unlock()
            {
            }

            typedef boost::unique_lock<NullLock> scoped_lock;

        private:
#ifdef FRAMEWORK_THREAD_NULL_LOCK_ASSERT
            boost::thread::id tid_;
#endif
		};

    } // namespace thread
} // namespace framework

#endif // #ifndef _FRAMEWORK_THREAD_NULL_LOCK_H_
