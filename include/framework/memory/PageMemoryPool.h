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

            static size_t page_per_block_; // ÿ��ҳ����
            static size_t size_block_; // ���С���ֽ���
            static size_t max_block_; // ������ʹ�õ��ڴ����
            static size_t num_block_; // �Ѿ�������ڴ����
            static size_t num_block_used_; // �Ѿ�ʹ�õ��ڴ����
            static Block * free_blocks_; // ���յ��ڴ��
            static boost::mutex * mutex_;
        };

        // ��ָ���࣬��ʼ��ΪNULL�������Զ��ͷ�ָ��
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
                // PageMemoryPoolû�����ü�����ֻ�����ϲ��ͷ�
                //if (blk_)
                //    PageMemoryPool::free(*this);
            }

            Ptr & operator = (
                Ptr const & ptr)
            {
                // PageMemoryPoolû�����ü�����ֻ�����ϲ��ͷ�
                //if (blk_)
                //    PageMemoryPool::free(*this);
                blk_ = ptr.blk_;
                return *this;
            }

            void release()
            {
                // PageMemoryPoolû�����ü�����ֻ�����ϲ��ͷ�
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
