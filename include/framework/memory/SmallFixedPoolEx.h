// SmallFixedEx.h

#ifndef _FRAMEWORK_MEMORY_SMALL_FIXED_POOL_EX_H_
#define _FRAMEWORK_MEMORY_SMALL_FIXED_POOL_EX_H_

#include "framework/memory/FixedPool.h"

namespace framework
{
    namespace memory
    {

        class SmallFixedEx
        {
        public:
            size_t change_size(
                size_t size_block_head, 
                size_t size_object_old, 
                size_t size_object) const
            {
                assert(size_object <= 128);
                size_t size_block = MemoryPage::page_size();
                return size_block;
            }

            struct Object
            {
                Object(
                    void * blk)
                    : blk_(blk)
                {
                }

                void * block() const
                {
                    return (void *)((size_t)this & ~(MemoryPage::page_size() - 1));
                }

            private:
                void * blk_;
            };
        };

        typedef FixedPool<SmallFixedEx> SmallFixedPoolEx;

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_SMALL_FIXED_POOL_EX_H_
