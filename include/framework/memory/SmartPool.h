// SmartPool.h

#ifndef _FRAMEWORK_MEMORY_SMART_POOL_H_
#define _FRAMEWORK_MEMORY_SMART_POOL_H_

#include "framework/container/UnidirList.h"
#include "framework/memory/PrivateMemory.h"

namespace framework
{
    namespace memory
    {

        struct SmartPool
            : PrivateMemory
        {
            SmartPool()
            {
            }

            ~SmartPool()
            {
                while (Block * blk = blocks_.first()) {
                    blocks_.pop();
                    PrivateMemory::free_block(blk, blk->size());
                }
            }

            struct Block
                : framework::container::UnidirListHook<Block>::type
            {
                char * cur;
                char * end;

                Block(
                    size_t size)
                    : cur((char *)(this + 1))
                    , end((char *)this + size)
                {
                }

                void * alloc(
                    size_t size)
                {
                    size_t * p = (size_t *)cur;
                    cur += size + sizeof(size_t);
                    *p = size;
                    return p + 1;
                }

                void * realloc(
                    void * ptr, 
                    size_t size)
                {
                    cur = (char *)ptr + size;
                    return ptr;
                }

                void free(
                    void * ptr)
                {
                    cur = (char *)ptr - sizeof(size_t);
                }

                bool can_alloc(
                    size_t size) const
                {
                    return cur + size + sizeof(size_t) <= end;
                }

                bool can_realloc(
                    void * ptr, 
                    size_t size) const
                {
                    size_t * p = (size_t *)ptr - 1;
                    return ((char *)ptr + *p == cur) && ((char *)ptr + size <= end);
                }

                bool can_free(
                    void * ptr)
                {
                    size_t * p = (size_t *)ptr - 1;
                    return (char *)ptr + *p == cur;
                }

                size_t size() const
                {
                    return end - (char *)this;
                }
            };

            void * alloc(
                size_t size)
            {
                size = MemoryPage::align_object(size);
                Block * blk = blocks_.first();
                if (blk == NULL || !blk->can_alloc(size)) {
                    size_t size_block = MemoryPage::align_page(size + sizeof(Block));
                    void * ptr = PrivateMemory::alloc_block(size_block);
                    if (ptr == NULL) {
                        return NULL;
                    }
                    blk = new (ptr) Block(size_block);
                    blocks_.push(blk);
                }
                return blk->alloc(size);
            }

            void * realloc(
                void * ptr, 
                size_t size)
            {
                Block * blk = blocks_.first();
                if (blk->can_realloc(ptr, size)) {
                    return blk->realloc(ptr, size);
                } else {
                    size_t * p = (size_t *)ptr - 1;
                    void * newptr = alloc(size);
                    memcpy(newptr, ptr, *p);
                    return newptr;
                }
            }

            void free(
                void * ptr)
            {
                Block * blk = blocks_.first();
                if (blk->can_free(ptr)) {
                    blk->free(ptr);
                }
            }

            typedef framework::container::UnidirList<Block> BlockList;
            BlockList blocks_;
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_SMART_POOL_H_
