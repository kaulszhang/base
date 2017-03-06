// PoolObject.h

#include "framework/memory/SmartPool.h"

#include <boost/thread/mutex.hpp>

namespace framework
{
    namespace memory
    {

        namespace detail
        {

            static SmartPool & smart_pool()
            {
                static SmartPool pool;
                return pool;
            }

            static boost::mutex & get_mutex()
            {
                static boost::mutex mtx;
                return mtx;
            }

            static SmartPool & smt_pool = smart_pool();

            struct PoolObject
            {
                void * operator new(
                    size_t size) throw ()
                {
                    boost::mutex::scoped_lock lock(get_mutex());
                    return smart_pool().alloc(size);
                }

                void operator delete(
                    void * ptr) throw ()
                {
                    boost::mutex::scoped_lock lock(get_mutex());
                    smart_pool().free(ptr);
                }
            };

        }

    } // namespace memory
} // namespace framework
