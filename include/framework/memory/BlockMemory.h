// BlockMemory.h

#ifndef _FRAMEWORK_MEMORY_BLOCK_MEMORY_H_
#define _FRAMEWORK_MEMORY_BLOCK_MEMORY_H_

namespace framework
{
    namespace memory
    {

        class BlockMemory
        {
        protected:
            typedef void * (*alloc_block_type)(
                BlockMemory & bm, 
                size_t size);

            typedef void (*free_block_type)(
                BlockMemory & bm, 
                void * ptr, 
                size_t size);

            typedef BlockMemory * (*clone_type)(
                BlockMemory & bm);

            typedef void (*destroy_type)(
                BlockMemory & bm);

        public:
            BlockMemory(
                alloc_block_type alloc_block, 
                free_block_type free_block, 
                clone_type clone, 
                destroy_type destroy)
                : alloc_block_(alloc_block)
                , free_block_(free_block)
                , clone_(clone)
                , destroy_(destroy)
            {
            }

        public:
            void * alloc_block(
                size_t size)
            {
                return alloc_block_(*this, size);
            }

            void free_block(
                void * ptr, 
                size_t size)
            {
                return free_block_(*this, ptr, size);
            }

            BlockMemory * clone()
            {
                return clone_(*this);
            }

            void destroy()
            {
                destroy_(*this);
            }

        private:
            alloc_block_type alloc_block_;
            free_block_type free_block_;
            clone_type clone_;
            destroy_type destroy_;
        };

        template <
            typename Memory
        >
        class BlockMemoryT
            : public BlockMemory
        {
        public:
            BlockMemoryT(
                Memory const & mem)
                : BlockMemory(
                    &alloc_block, 
                    &free_block, 
                    &clone, 
                    &destroy)
                , mem_(mem)
            {
            }

        private:
            static void * alloc_block(
                BlockMemory & base, 
                size_t size)
            {
                BlockMemoryT & me = static_cast<BlockMemoryT &>(base);
                return me.mem_.alloc_block(size);
            }

            static void free_block(
                BlockMemory & base, 
                void * ptr, 
                size_t size)
            {
                BlockMemoryT & me = static_cast<BlockMemoryT &>(base);
                me.mem_.free_block(ptr, size);
            }

            static BlockMemory * clone(
                BlockMemory & base)
            {
                BlockMemoryT & me = static_cast<BlockMemoryT &>(base);
                return new BlockMemoryT(me.mem_);
            }

            static void destroy(
                BlockMemory & base)
            {
                BlockMemoryT & me = static_cast<BlockMemoryT &>(base);
                delete &me;
            }

        public:
            void * operator new(
                size_t size)
            {
                return malloc(size);
            }

            void operator delete(
                void * ptr)
            {
                free(ptr);
            }

        private:
            Memory mem_;
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_BLOCK_MEMORY_H_
