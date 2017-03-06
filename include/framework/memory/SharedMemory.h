// SharedMemory.h

#ifndef _FRAMEWORK_MEMORY_SHARED_MEMORY_H_
#define _FRAMEWORK_MEMORY_SHARED_MEMORY_H_

#include "framework/memory/MemoryPage.h"
#include "framework/generic/Pointer.h"

#define SHARED_MEMORY_MAX_INST_ID	16
#define SHARED_MEMORY_MAX_USER_ID	16
#define SHARED_MEMORY_VERSION       1

namespace framework
{
    namespace memory
    {

        namespace detail
        {
            class SharedMemoryImpl;
        }

        class SharedMemory
            : public MemoryPage
        {
        public:
            enum ImplEnum
            {
#ifdef BOOST_WINDOWS_API
                windows, 
                win_file, 
#else 
#  ifndef FRAMEWORK_NO_SYSTEM_V_IPC
                systemv, 
#  endif
#  ifndef FRAMEWORK_NO_POSIX_IPC
                posix, 
#  endif
                file, 
#endif
                private_, 
                default_ = 0, 
            };

            enum FlagEnum
            {
                create = 1 << 8, 
                exclusive = 2 << 8, 
                alloc_get = 4 << 8, 
                read_only = 16 << 8, 
                read_write = 48 << 8, 
            };

            static boost::uint32_t const no_user_id = (boost::uint32_t)-1;

        public:
            static boost::system::error_code remove(
                boost::uint32_t iid);

            static size_t min_block_size()
            {
                return page_size();
            }

        public:
            SharedMemory(
                boost::uint32_t iid = SHARED_MEMORY_MAX_INST_ID);

            SharedMemory(
                boost::uint32_t iid, 
                boost::uint32_t uid, 
                boost::uint32_t flag, 
                boost::system::error_code & ec);

            ~SharedMemory();

        public:
            boost::system::error_code open(
                boost::uint32_t iid, 
                boost::uint32_t uid, 
                boost::uint32_t flag, 
                boost::system::error_code & ec);

            boost::system::error_code open(
                boost::uint32_t uid, 
                boost::uint32_t flag, 
                boost::system::error_code & ec);

            boost::system::error_code open(
                boost::uint32_t flag, 
                boost::system::error_code & ec);

            bool is_open() const;

            boost::system::error_code close(
                boost::system::error_code & ec);

        public:
            size_t block_size(
                size_t size);

            size_t block_size() const;

        public:
            void * alloc_with_id(
                size_t id, 
                size_t size);

            void * get_by_id(
                size_t id);

            void * addr_ref_to_store(
                boost::uint32_t uid, 
                void * addr);

            void * addr_store_to_ref(
                boost::uint32_t uid, 
                void * addr);

            bool addr_ref_to_store(
                void * addr, 
                boost::uint32_t & key, 
                size_t & offset);

            bool addr_store_to_ref(
                void *& addr, 
                boost::uint32_t key, 
                size_t offset);

        public:
            static void * get_by_id(
                boost::uint32_t iid, 
                size_t id);

            static void * addr_ref_to_store(
                boost::uint32_t iid, 
                boost::uint32_t uid, 
                void * addr);

            static void * addr_store_to_ref(
                boost::uint32_t iid, 
                boost::uint32_t uid, 
                void * addr);

            static bool addr_ref_to_store(
                boost::uint32_t iid, 
                void * addr, 
                boost::uint32_t & key, 
                size_t & offset);

            static bool addr_store_to_ref(
                boost::uint32_t iid, 
                void *& addr, 
                boost::uint32_t key, 
                size_t offset);

        public:
            void * alloc_block(
                size_t size);

            void free_block(
                void * ptr, 
                size_t size);

        private:
            class SharedPointer;
            struct Block;
            struct BlockItem;
            struct BlockList;
            struct LocalBlock;
            struct IdObject;
            struct Head;
            struct AllocPos;
            struct LocalBlockList;

        private:
            LocalBlock * create_shm(
                boost::system::error_code & ec);

            LocalBlock * open_shm(
                boost::system::error_code & ec);

            void open_for_remove(
                boost::uint32_t iid, 
                boost::system::error_code & ec);

            void open_for_remove(
                boost::uint32_t iid, 
                boost::uint32_t flag, 
                boost::system::error_code & ec);

            void check(
                BlockItem const & shared_list, 
                LocalBlockList & local_list);

            void * alloc(
                size_t size, 
                boost::system::error_code & ec);

            LocalBlock * alloc_raw_block(
                boost::uint32_t key, 
                boost::uint32_t size, 
                boost::system::error_code & ec);

            LocalBlock * open_raw_block(
                boost::uint32_t key, 
                boost::uint32_t size, 
                boost::system::error_code & ec);

            void free_raw_block(
                LocalBlock const * b, 
                bool & r, 
                boost::system::error_code & ec);

            bool valid_alloc(
                boost::uint32_t size, 
                boost::system::error_code & ec);

            void insert_block(
                BlockItem & list, 
                LocalBlock & lb);

            void remove_block(
                BlockItem & list, 
                LocalBlock & lb);

        private:
            static SharedMemory * instance_[SHARED_MEMORY_MAX_INST_ID];

        private:
            Head * head_;
            boost::uint32_t inst_id_; // 我的User ID
            boost::uint32_t user_id_; // 我的User ID
            boost::uint32_t flag_;
            detail::SharedMemoryImpl * impl_;
            LocalBlockList * local_main_blocks_;
            LocalBlockList * local_blocks_;
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_SHARED_MEMORY_POOL_H_
