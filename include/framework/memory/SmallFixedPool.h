// SmallFixedPool.h

#ifndef _FRAMEWORK_MEMORY_SMALL_FIXED_POOL_H_
#define _FRAMEWORK_MEMORY_SMALL_FIXED_POOL_H_

#include "framework/memory/FixedPool.h"

namespace framework
{
    namespace memory
    {

        class SmallFixed
        {
        public:
            size_t change_size(
                size_t size_block_head, 
                size_t size_object_old, 
                size_t size_object) const
            {
                if (size_object_old)
                    return 0;
                assert(size_object <= 128);
                size_t size_block = MemoryPage::page_size();
                return size_block;
            }

            struct Object
            {
                Object(
                    void *)
                {
                }

                void * block() const
                {
                    return (void *)((size_t)this & ~(MemoryPage::page_size() - 1));
                }
            };
        };

        typedef FixedPool<SmallFixed> SmallFixedPool;

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_SMALL_FIXED_POOL_H_
