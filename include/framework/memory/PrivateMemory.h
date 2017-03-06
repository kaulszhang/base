// PrivateMemory.h

#ifndef _FRAMEWORK_MEMORY_PRIVATE_MEMORY_H_
#define _FRAMEWORK_MEMORY_PRIVATE_MEMORY_H_

#include "framework/memory/MemoryPage.h"
#include "framework/container/OrderedUnidirList.h"
using namespace framework::container;
using namespace framework::generic;

namespace framework
{
    namespace memory
    {

        struct PriMemItem
            : OrderedUnidirListHook< PriMemItem, NativePointerTraits<PriMemItem> >::type
        {
            PriMemItem(
                size_t n, size_t size )
                : n_(n)
                , size_(size)
            {
            }

            bool operator < (
                PriMemItem const & r) const
            {
                return n_ < r.n_;
            }

            size_t n_;
            size_t size_;
        };

        class PrivateMemory
            : public MemoryPage
        {
        public:
            PrivateMemory()
                : private_pool_( NULL )
            {
            }

        public:
            static size_t min_block_size()
            {
                return page_size();
            }

        public:
            void * alloc_block(
                size_t size);

            void free_block(
                void * addr, 
                size_t size);

        public:
            void * alloc_with_id(
                size_t id, 
                size_t size);

            void * get_by_id(
                size_t id);

            void close( boost::system::error_code & ec );

        private:
            typedef struct _tInternalData
            {
                _tInternalData()
                    : alloc_pos_( 0 )
                {
                    primems_.clear();
                }

                OrderedUnidirList< PriMemItem > primems_;
                size_t alloc_pos_;
            } InternalData;
            InternalData * private_pool_;
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_PRIVATE_MEMORY_H_
