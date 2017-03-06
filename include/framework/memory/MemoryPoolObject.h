// MemoryPoolObject.h

#ifndef _FRAMEWORK_MEMORY_MEMORY_POOL_OBJECT_H_
#define _FRAMEWORK_MEMORY_MEMORY_POOL_OBJECT_H_

#include "framework/thread/NullLock.h"
#include "framework/memory/PrivateMemory.h"
#include "framework/memory/BigFixedPool.h"

namespace framework
{
    namespace memory
    {

        // 申请不到内存，不抛出异常
        template <
            typename _Ty, 
            typename _Pl = framework::memory::BigFixedPool, 
            typename _Lk = framework::thread::NullLock
        >
        struct MemoryPoolObjectNoThrow
        {
        public:
            typedef _Ty object_type;

            typedef _Pl pool_type;

            typedef _Lk lock_type;

            typedef MemoryPoolObjectNoThrow pool_object_type;

        public:
            MemoryPoolObjectNoThrow()
            {
            }

            template <typename _Ty1>
            MemoryPoolObjectNoThrow(
                MemoryPoolObjectNoThrow<_Ty1, _Pl, _Lk> const & r)
            {
            }

        public:
            static void set_pool(
                _Pl const & p)
            {
                pool().~_Pl();
                new (&pool())_Pl(p);
            }

            static _Pl const & get_pool()
            {
                return pool();
            }

            static void clear_pool()
            {
                pool().clear();
            }

            static size_t set_pool_capacity(
                size_t size)
            {
                return pool().capacity(size);
            }

        public:
            void * operator new(
                size_t size) throw ()
            {
                return alloc(size);
            }

            void operator delete(
                void * ptr) throw ()
            {
                free(ptr);
            }

        public:
            template <typename _Ty1>
            struct rebind
            {
                typedef MemoryPoolObjectNoThrow<_Ty1, _Pl, _Lk> type;
            };

        private:
            void *operator new[] (size_t)  throw ();

            void operator delete[] (void *)  throw ();

        protected:
            static void * alloc(
                size_t size)
            {
                assert(size == sizeof(_Ty));
                typename _Lk::scoped_lock scoped_lock(lock());
                return pool().alloc(size);
            }

            static void free(
                void * ptr)
            {
                typename _Lk::scoped_lock scoped_lock(lock());
                return pool().free(ptr);
            }

        protected:
            static _Pl init_pool()
            {
                return _Pl(PrivateMemory());
            }

            static _Pl & pool()
            {
                static _Pl pl(_Ty::init_pool());
                return pl;
            }

            static _Lk & lock()
            {
                static _Lk lk;
                return lk;
            }
        };

        // 申请不到内存，抛出异常
        template <
            typename _Ty, 
            typename _Pl = framework::memory::BigFixedPool, 
            typename _Lk = framework::thread::NullLock
        >
        struct MemoryPoolObject
            : MemoryPoolObjectNoThrow<_Ty, _Pl, _Lk>
        {
        public:
            void * operator new(
                size_t size) throw (std::bad_alloc)
            {
                void * addr = MemoryPoolObjectNoThrow<_Ty, _Pl, _Lk>::alloc(size);
                if (addr == NULL)
                    throw std::bad_alloc();
                return addr;
            }

        private:
            void *operator new[] (size_t)  throw (std::bad_alloc);

        public:
            template <typename _Ty1>
            struct rebind
            {
                typedef MemoryPoolObject<_Ty1, _Pl, _Lk> type;
            };
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_MEMORY_POOL_OBJECT_H_
