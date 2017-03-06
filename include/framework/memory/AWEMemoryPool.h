// AWEMomeryPool.h

#ifndef __FRAMEWORK_MEMORY_PAGE_ALLOC_H_AWE_MEMORY_H_
#define __FRAMEWORK_MEMORY_PAGE_ALLOC_H_AWE_MEMORY_H_

#if defined _WIN32 && _WIN32_WINNT >= 0x0500

namespace boost
{
    class mutex;
}

namespace framework
{
    namespace memory
    {

        class AWEMemoryPool
        {
        public:
            class WeakPtr;
            class SharePtr;

        public:
            static int init(
                size_t size, 
                size_t num);

            static void term(void);

        public:
            static WeakPtr alloc();

            static void free(
                WeakPtr & ptr);

            static size_t num_used()
            {
                return num_physical_block_used_;
            }

            static size_t max_used()
            {
                return max_physical_block_used_;
            }

            static size_t capcity()
            {
                return num_physical_block_;
            }

            static size_t num_used_virtual()
            {
                return num_virtual_block_used_;
            }

            static size_t max_used_virtual()
            {
                return max_virtual_block_used_;
            }

        private:
            struct Block
            {
                void * pfns;
                void * addr;
                Block * next;
                union {
                    Block * prev;
                    struct  
                    {
                        short ref;
                        short hold;
                    };
                };
            };

            static void hold(
                Block * blk);

            static void unhold(
                Block * blk);

            static void ref(
                Block * blk);

            static void unref(
                Block * blk);

            friend class WeakPtr;
            friend class SharePtr;

            static size_t page_per_block_; // 每块页面数
            static size_t size_block_; // 块大小，字节数
            static size_t num_physical_block_; // 物理块数
            static size_t num_virtual_block_; // 虚拟内存块数
            static size_t num_physical_block_used_; // 已经使用的物理内存块数
            static size_t max_physical_block_used_; // 最大使用的物理内存块数
            static size_t num_virtual_block_used_; // 已经使用的虚拟内存块数
            static size_t max_virtual_block_used_; // 最大使用的虚拟内存块数
            static void * page_nums_; // 页号
            static void * virtual_addr_;
            static Block * physical_blocks_;
            static Block * virtual_blocks_;
            static Block * free_physical_blocks_;
            static Block free_virtual_blocks_;
            static boost::mutex * mutex_;
        };

        class AWEMemoryPool::WeakPtr
        {
        public:
            WeakPtr()
                : blk_(NULL)
            {
            }

            WeakPtr(
                WeakPtr const & ptr)
                : blk_(ptr.blk_)
            {
                if (blk_)
                    AWEMemoryPool::hold(blk_);
            }

            ~WeakPtr()
            {
                if (blk_)
                    AWEMemoryPool::unhold(blk_);
            }

            WeakPtr & operator = (
                WeakPtr const & ptr)
            {
                if (blk_)
                    AWEMemoryPool::unhold(blk_);
                blk_ = ptr.blk_;
                if (blk_)
                    AWEMemoryPool::hold(blk_);
                return *this;
            }

            void release()
            {
                if (blk_)
                    AWEMemoryPool::unhold(blk_);
                blk_ = NULL;
            }

            operator bool () const
            {
                return blk_ != 0;
            }

            friend class AWEMemoryPool;

        protected:
            WeakPtr(
                Block * blk)
                : blk_(blk)
            {
            }

            Block * blk_;
        };

        class AWEMemoryPool::SharePtr
            : public WeakPtr
        {
        public:
            SharePtr()
            {
            }

            SharePtr(
                WeakPtr const & wptr)
                : WeakPtr(wptr)
            {
                if (blk_)
                    AWEMemoryPool::ref(blk_);
            }

            SharePtr(
                SharePtr const & sptr)
                : WeakPtr(sptr)
            {
                if (blk_)
                    AWEMemoryPool::ref(blk_);
            }

            ~SharePtr()
            {
                if (blk_)
                    AWEMemoryPool::unref(blk_);
            }

            SharePtr & operator = (
                WeakPtr const & ptr)
            {
                if (blk_)
                    AWEMemoryPool::unref(blk_);
                WeakPtr::operator = (ptr);
                if (blk_)
                    AWEMemoryPool::ref(blk_);
                return *this;
            }

            operator void * () const
            {
                return blk_ ? blk_->addr : NULL;
            }

            template <typename T>
            operator T * () const
            {
                return blk_ ? static_cast<T *>(blk_->addr) : NULL;
            }
        };

    } // namespace memory
} // namespace framework

#endif // _WIN32

#endif // __FRAMEWORK_MEMORY_PAGE_ALLOC_H_AWE_MEMORY_H_
