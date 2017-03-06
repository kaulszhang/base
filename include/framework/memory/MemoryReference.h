// MemoryReference.h

#ifndef _FRAMEWORK_MEMORY_MEMORY_REFERENCE_H_
#define _FRAMEWORK_MEMORY_MEMORY_REFERENCE_H_

namespace framework
{
    namespace memory
    {

        template <
            typename _M
        >
        class MemoryReference
            : public MemoryPage
        {
        public:
            MemoryReference(
                _M & m)
                : m_(m)
            {
            }

        public:
            void * alloc_block(
                size_t size)
            {
                return m_.alloc_block(size);
            }

            void free_block(
                void * addr, 
                size_t size)
            {
                return m_.free_block(addr, size);
            }

        private:
            _M & m_;
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_MEMORY_REFERENCE_H_
