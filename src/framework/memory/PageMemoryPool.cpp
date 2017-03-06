// PageMemoryPool.cpp

#include "framework/Framework.h"
#include "framework/memory/PageMemoryPool.h"

#include <boost/thread/mutex.hpp>

namespace framework
{
    namespace memory
    {

        size_t PageMemoryPool::page_per_block_ = 1; // 每块页面数
        size_t PageMemoryPool::size_block_ = 0; // 块大小，字节数
        size_t PageMemoryPool::max_block_ = 1000; // 最大可以使用的内存块数
        size_t PageMemoryPool::num_block_ = 0; // 已经申请的内存块数
        size_t PageMemoryPool::num_block_used_ = 0; // 已经使用的内存块数
        PageMemoryPool::Block * PageMemoryPool::free_blocks_ = NULL;
        boost::mutex * PageMemoryPool::mutex_ = NULL;

        int PageMemoryPool::init(
            size_t size, 
            size_t num)
        {
            max_block_ = num;

            size_t page_size = PageMemoryPool::page_size();

            page_per_block_ = (size - 1) / page_size + 1;
            size_block_ = page_per_block_ * page_size;

            mutex_ = new boost::mutex;

            return 0;
        }

        void PageMemoryPool::term()
        {
            delete mutex_;
        }

        PageMemoryPool::Ptr PageMemoryPool::alloc()
        {
            boost::mutex::scoped_lock lock(*mutex_);

            if (!free_blocks_) {
                if (num_block_ >= max_block_)
                    return NULL;
                free_blocks_ = (Block *)alloc_block(size_block_);
                //
                if (!free_blocks_) {
                    return NULL;
                } else {
                    free_blocks_->next = NULL;
                    num_block_++;
                }
            }

            Block * ptr = free_blocks_;
            free_blocks_ = free_blocks_->next;
            num_block_used_++;
            return ptr;
        }

        void PageMemoryPool::free(
            Ptr & ptr)
        {
            boost::mutex::scoped_lock lock(*mutex_);

            Block * blk = ptr.blk_;
            ptr.blk_ = NULL;

            blk->next = free_blocks_;
            free_blocks_ = blk;
            num_block_used_--;
        }

    } // namespace memory
} // namespace framework
