// BigFixedPool.h

#ifndef _FRAMEWORK_MEMORY_BIG_FIXED_POOL_H_
#define _FRAMEWORK_MEMORY_BIG_FIXED_POOL_H_

#include "framework/memory/FixedPool.h"

namespace framework
{
    namespace memory
    {

        class BigFixed
        {
        public:
            size_t change_size(
                size_t size_block_head, 
                size_t size_object_old, 
                size_t size_object) const
            {
                if (size_object_old > 0)
                    return 0;
                size_t size_block = MemoryPage::page_size();
                size_t object_per_block = (size_block - size_block_head) / size_object;
                while (object_per_block < 128) {
                    size_block *= 2;
                    object_per_block = (size_block - size_block_head) / size_object;
                }
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
                    return blk_;
                }

            private:
                void * blk_;
            };
        };

        typedef FixedPool<BigFixed> BigFixedPool;

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_BIG_FIXED_POOL_H_
