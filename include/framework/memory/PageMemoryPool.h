// PageMemoryPool.h

#ifndef _FRAMEWORK_MEMORY_PAGE_MEMORYPOOL_H_
#define _FRAMEWORK_MEMORY_PAGE_MEMORYPOOL_H_

#include "framework/memory/PrivateMemory.h"

namespace boost
{
    class mutex;
}

namespace framework
{
    namespace memory
    {

        class PageMemoryPool
            : private PrivateMemory
        {
        public:
            class Ptr;

        public:
            static int init(
                size_t size, 
                size_t num);

            static void term();

        public:
            Ptr alloc();

            void free(
                Ptr & ptr);

            size_t num_used()
            {
                return num_block_used_;
            }

            size_t max_used()
            {
                return num_block_;
            }

            size_t capcity()
            {
                return max_block_;
            }

        private:
            struct Block
            {
                Block * next;
            };

            static size_t page_per_block_; // 每块页面数
            static size_t size_block_; // 块大小，字节数
            static size_t max_block_; // 最大可以使用的内存块数
            static size_t num_block_; // 已经申请的内存块数
            static size_t num_block_used_; // 已经使用的内存块数
            static Block * free_blocks_; // 回收的内存块
            static boost::mutex * mutex_;
        };

        // 简单指针类，初始化为NULL，不会自动释放指针
        class PageMemoryPool::Ptr
        {
        public:
            Ptr()
                : blk_(NULL)
            {
            }

            Ptr(
                Ptr const & ptr)
                : blk_(ptr.blk_)
            {
            }

            ~Ptr()
            {
                // PageMemoryPool没有引用计数，只能由上层释放
                //if (blk_)
                //    PageMemoryPool::free(*this);
            }

            Ptr & operator = (
                Ptr const & ptr)
            {
                // PageMemoryPool没有引用计数，只能由上层释放
                //if (blk_)
                //    PageMemoryPool::free(*this);
                blk_ = ptr.blk_;
                return *this;
            }

            void release()
            {
                // PageMemoryPool没有引用计数，只能由上层释放
                //if (blk_)
                //    PageMemoryPool::free(*this);
                blk_ = NULL;
            }

            operator bool () const
            {
                return blk_ != 0;
            }

            operator void * () const
            {
                return blk_;
            }

            template <typename T>
            operator T * () const
            {
                return reinterpret_cast<T *>(blk_);
            }

        protected:
            friend class PageMemoryPool;

            Ptr(
                Block * blk)
                : blk_(blk)
            {
            }

            Block * blk_;
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_PAGE_MEMORYPOOL_H_
