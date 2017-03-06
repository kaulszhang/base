// MemoryPool.h

#ifndef _FRAMEWORK_MEMORY_MEMORY_POOL_H_
#define _FRAMEWORK_MEMORY_MEMORY_POOL_H_

#include "framework/memory/MemoryPage.h"
#include "framework/memory/BlockMemory.h"

#if (defined NDEBUG || defined _NDEBUG)
#  if (defined FRAMEWORK_MEMORY_DEBUG_MEMORY_POOL)
#    undef FRAMEWORK_MEMORY_DEBUG_MEMORY_POOL
#  endif
#endif

#ifdef FRAMEWORK_MEMORY_DEBUG_MEMORY_POOL
#  include "framework/memory/MemoryPoolDebug.h"
#endif

namespace framework
{
    namespace memory
    {

        struct AllocatorBind;

        class MemoryPool
        {
        protected:
            MemoryPool(
                BlockMemory & bm, 
                size_t capacity)
                : bm_(&bm)
                , capacity_(capacity)
                , consumption_(0)
                , num_block_(0)
                , num_object_(0)
            {
            }

            template <
                typename _Mm
            >
            MemoryPool(
                _Mm const & mm, 
                size_t capacity)
                : bm_(new BlockMemoryT<_Mm>(mm))
                , capacity_(capacity)
                , consumption_(0)
                , num_block_(0)
                , num_object_(0)
            {
            }

            MemoryPool(
                MemoryPool const & r)
                : bm_(r.bm_->clone())
                , capacity_(r.capacity_)
                , consumption_(0)
                , num_block_(0)
                , num_object_(0)
#ifdef FRAMEWORK_MEMORY_DEBUG_MEMORY_POOL
                , debug_(r.debug_)
#endif
            {
            }

            ~MemoryPool()
            {
                bm_->destroy();
            }

        public:
            size_t capacity(
                size_t size)
            {
                size_t old = capacity_;
                capacity_ = size;
                return old;
            }

            size_t capacity() const
            {
                return capacity_;
            }

            size_t consumption() const
            {
                return consumption_;
            }

            size_t peek() const
            {
                return peek_;
            }

            size_t num_block() const
            {
                return num_block_;
            }

            size_t num_object() const
            {
                return num_object_;
            }

        public:
            void debug_dump()
            {
#ifdef FRAMEWORK_MEMORY_DEBUG_MEMORY_POOL
                debug_.dump();
#endif
            }

        protected:
            template <
                typename ObjectHead
            >
            struct Object
                : ObjectHead
            {
#ifdef FRAMEWORK_MEMORY_DEBUG_MEMORY_POOL
                MemoryPoolDebug::DebugInfo debug_info;
#endif

                Object()
                {
                }

                template <
                    typename _Arg1
                >
                Object(
                    _Arg1 arg1)
                    : ObjectHead(arg1)
                {
                }

                template <
                    typename _Arg1, 
                    typename _Arg2
                >
                Object(
                    _Arg1 arg1, 
                    _Arg2 arg2)
                    : ObjectHead(arg1, arg2)
                {
                }
            };

        protected:
            void * alloc_block(
                size_t size);

            void free_block(
                void * ptr, 
                size_t size);

            template <
                typename ObjectHead
            >
            void alloc_object(
                Object<ObjectHead> * obj)
            {
                ++num_object_;
#ifdef FRAMEWORK_MEMORY_DEBUG_MEMORY_POOL
                debug_.mark(obj->debug_info);
#endif
            }

            template <
                typename ObjectHead
            >
            void free_object(
                Object<ObjectHead> * obj)
            {
                --num_object_;
#ifdef FRAMEWORK_MEMORY_DEBUG_MEMORY_POOL
                debug_.unmark(obj->debug_info);
#endif
            }

            template <
                typename ObjectHead
            >
            void leak_object(
                Object<ObjectHead> * obj, 
                void * ptr, 
                size_t size)
            {
#ifdef FRAMEWORK_MEMORY_DEBUG_MEMORY_POOL
                debug_.dump(obj->debug_info, ptr, size - sizeof(Object<ObjectHead>));
#endif
            }

        private:
            BlockMemory * bm_;
            size_t capacity_;
            size_t consumption_;
            size_t num_block_;
            size_t num_object_;
            size_t peek_;
#ifdef FRAMEWORK_MEMORY_DEBUG_MEMORY_POOL
            MemoryPoolDebug debug_;
#endif
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_MEMORY_POOL_H_
