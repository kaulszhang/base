#ifndef _FRAMEWORK_MEMORY_MEMORY_PAGE_H_
#define _FRAMEWORK_MEMORY_MEMORY_PAGE_H_

#ifndef FRAMEWORK_MEMORY_ALIGNMENT
#  define FRAMEWORK_MEMORY_ALIGNMENT 8
#endif

namespace framework
{
    namespace memory
    {

        class MemoryPage
        {
        public:
            BOOST_STATIC_CONSTANT(size_t, alignment = FRAMEWORK_MEMORY_ALIGNMENT);

        public:
            static size_t page_size();

            static size_t align_page(
                size_t size)
            {
                if (size & (page_size() - 1))
                    size = (size & (~(page_size() - 1))) + page_size();
                return size;
            }

            static size_t align_object(
                size_t size)
            {
                // 整形大小对齐
                if (size & (alignment - 1))
                    size = (size & (~(alignment - 1))) + alignment;
                return size;
            }

            template <
                size_t size
            >
            struct static_align_object
            {
                static size_t const value = 
                    (size & (alignment - 1)) ? (size & (~(alignment - 1))) + alignment : size;
            };
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_MEMORY_PAGE_H_
