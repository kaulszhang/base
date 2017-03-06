// SharedMemory.cpp

#include "framework/Framework.h"
#include "framework/memory/SharedMemory.h"
#include "framework/process/Mutex.h"
#include "framework/string/Format.h"
#include "framework/system/ErrorCode.h"
#include "framework/container/List.h"
#include "framework/logger/Logger.h"
#include "framework/logger/LoggerFormatRecord.h"
#include "framework/Version.h"
using namespace framework::logger;
using namespace framework::system;
using namespace framework::string;
using namespace framework::process;

#include <boost/thread/thread.hpp>
using namespace boost::system;

#include <iostream>

#include "framework/memory/detail/SharedMemoryImpl.h"
#ifdef BOOST_WINDOWS_API
#  include "framework/memory/detail/SharedMemoryWindows.h"
#  include "framework/memory/detail/SharedMemoryWinFile.h"
#else 
#  ifndef FRAMEWORK_NO_POSIX_IPC
#    include "framework/memory/detail/SharedMemoryPosix.h"
#  endif
#  include "framework/memory/detail/SharedMemoryFile.h"
#  ifndef FRAMEWORK_NO_SYSTEM_V_IPC
#    include "framework/memory/detail/SharedMemorySystemV.h"
#  endif
#endif
#  include "framework/memory/detail/SharedMemoryPrivate.h"

#define KEY_START   235562
#define NAME_LEN    64

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("SharedMemory", 2)

namespace framework
{
    namespace memory
    {

        static detail::SharedMemoryImpl * shared_memory_impls[] = {
#ifdef BOOST_WINDOWS_API
            &detail::shared_memory_windows, 
            &detail::shared_memory_win_file, 
#else 
#  ifndef FRAMEWORK_NO_SYSTEM_V_IPC
            &detail::shared_memory_systemv, 
#  endif
#  ifndef FRAMEWORK_NO_POSIX_IPC
            &detail::shared_memory_posix, 
#  endif
            &detail::shared_memory_file, 
#endif
            &detail::shared_memory_private, 
        };

        SharedMemory * SharedMemory::instance_[SHARED_MEMORY_MAX_INST_ID] = {NULL};

        class SharedMemory::SharedPointer
        {
        public:
            SharedPointer(
                SharedMemory * shm, 
                void * addr)
            {
                shm->addr_ref_to_store(addr, key_, off_);
            }

            SharedPointer(
                boost::uint32_t key = 0, 
                boost::uint32_t off = 0)
                : key_(key)
                , off_(off)
            {
            }

            SharedPointer & operator+=(
                size_t size)
            {
                off_ += size;
                return *this;
            }

            friend SharedPointer operator+(
                SharedPointer const & l, 
                size_t r)
            {
                return SharedPointer(l.key_, l.off_ + r);
            }

            friend size_t operator-(
                SharedPointer const & l, 
                SharedPointer const & r)
            {
                assert(l.key_ == r.key_);
                return l.off_ - r.off_;
            }

            friend bool operator==(
                SharedPointer const & l, 
                SharedPointer const & r)
            {
                return l.key_ == r.key_
                    && l.off_ == r.off_;
            }

            friend bool operator!=(
                SharedPointer const & l, 
                SharedPointer const & r)
            {
                return !(l == r);
            }

        public:
            void * addr_of(
                SharedMemory * shm) const
            {
                void * addr = NULL;
                shm->addr_store_to_ref(addr, key_, off_);
                return addr;
            }

        private:
            boost::uint32_t key_;
            size_t off_;
        };

        struct SharedMemory::Block
        {
            Block()
                : size(0)
                , key(0)
                , addr(NULL)
            {
            }

            boost::uint32_t size;
            boost::uint32_t key;
            // 描述内存块时，指内存块的地址（申请者的地址空间）
            void * addr;
        };

        struct SharedMemory::BlockItem
            : Block
        {
            BlockItem()
            {
            }

            // 指下一个Block结构的地址
            SharedPointer next;
            SharedPointer last;
        };

        struct SharedMemory::LocalBlock
            : framework::container::ListHook<LocalBlock>::type
        {
            LocalBlock()
                : block(NULL)
                , map_id(0)
                , map_addr(NULL)
            {
            }

            Block const * block;
            void * map_id;
            size_t size;
            void * map_addr;
        };

        struct SharedMemory::LocalBlockList
            : framework::container::List<SharedMemory::LocalBlock>
        {
            LocalBlockList()
                : next_to_open(NULL)
                , last_use(NULL)
            {
            }

            BlockItem const * next_to_open;
            LocalBlock const * last_use;
        };

        struct SharedMemory::IdObject
        {
            IdObject()
                : id(0)
            {
            }

            SharedPointer next;
            boost::uint32_t id;
            boost::uint32_t size;
        };

        struct SharedMemory::AllocPos
        {
            SharedPointer addr;
            SharedPointer end;
        };

        struct SharedMemory::Head
        {
            Head(
                boost::uint32_t iid, 
                boost::uint32_t flag)
                : iid(iid)
                , version(SHARED_MEMORY_VERSION)
                , flag(flag)
                , next_key(KEY_START)
                , min_block_size(SharedMemory::page_size())
            {
                memset(attach, 0, sizeof(attach));
            }

            boost::uint32_t iid;
            boost::uint32_t version;
            boost::uint32_t flag;
            boost::uint32_t next_key;
            boost::uint32_t min_block_size;
            framework::process::Mutex mutex;
            BlockItem main_blocks;
            // 每个使用者申请的内存块，链表组织，本结构是头部，不代表某个块
            BlockItem user_blocks[SHARED_MEMORY_MAX_USER_ID];
            boost::uint32_t attach[SHARED_MEMORY_MAX_USER_ID];
            AllocPos alloc_pos;
            IdObject objects;
            // 第一个内存块的描述结构
            //BlockItem first_block;
        };

        error_code SharedMemory::remove(
            boost::uint32_t iid)
        {
            boost::system::error_code ec;
            SharedMemory().open_for_remove(iid, ec);
            return ec;
        }

        SharedMemory::SharedMemory(
            boost::uint32_t iid)
            : head_(NULL)
            , inst_id_(iid)
            , user_id_(SHARED_MEMORY_MAX_USER_ID)
            , flag_(0)
            , impl_(NULL)
            , local_main_blocks_(NULL)
            , local_blocks_(NULL)
        {
        }

        SharedMemory::SharedMemory(
            boost::uint32_t iid, 
            boost::uint32_t uid, 
            boost::uint32_t flag, 
            error_code & ec)
            : head_(NULL)
            , user_id_(SHARED_MEMORY_MAX_USER_ID)
            , flag_(0)
            , impl_(NULL)
            , local_main_blocks_(NULL)
            , local_blocks_(NULL)
        {
            open(iid, uid, flag, ec);
        }

        SharedMemory::~SharedMemory()
        {
            if (!head_) {
                return;
            }
            error_code ec;
            close(ec);
        }

        error_code SharedMemory::open(
            boost::uint32_t uid, 
            boost::uint32_t flag, 
            error_code & ec)
        {
            return open(inst_id_, uid, flag, ec);
        }

        error_code SharedMemory::open(
            boost::uint32_t flag, 
            error_code & ec)
        {
            return open(inst_id_, no_user_id, flag, ec);
        }

        error_code SharedMemory::open(
            boost::uint32_t iid, 
            boost::uint32_t uid, 
            boost::uint32_t flag, 
            error_code & ec)
        {
            assert(iid < SHARED_MEMORY_MAX_INST_ID);
            assert(uid < SHARED_MEMORY_MAX_USER_ID || uid == no_user_id);
            assert(!is_open());

            inst_id_ = iid;
            user_id_ = uid;
            flag_ = flag;

            assert((flag_ & 0xff) < sizeof(shared_memory_impls) / sizeof(shared_memory_impls[0]));
            impl_ = shared_memory_impls[flag_ & 0xff];

            LocalBlock * lb = NULL;
            if (flag_ & create) {
                lb = create_shm(ec);
                if (!lb && !(flag_ & exclusive)) {
                    lb = open_shm(ec);
                }
            } else {
                lb = open_shm(ec);
            }

            if (lb) {
                local_main_blocks_ = new LocalBlockList;
                local_main_blocks_->push_back(lb);
                local_main_blocks_->next_to_open = (BlockItem const *)(head_ + 1);
                local_blocks_ = new LocalBlockList[SHARED_MEMORY_MAX_USER_ID];
                for (size_t uid = 0; uid < SHARED_MEMORY_MAX_USER_ID; ++uid) {
                    local_blocks_[uid].next_to_open = &head_->user_blocks[uid];
                }

                // attach
                if (user_id_ < SHARED_MEMORY_MAX_USER_ID)
                    head_->attach[user_id_] = (boost::uint32_t)time(NULL);
                instance_[iid] = this;
            }

            return ec;
        }

        bool SharedMemory::is_open() const
        {
            return head_ != NULL;
        }

        SharedMemory::LocalBlock * SharedMemory::create_shm(
            error_code & ec)
        {
            LocalBlock * lb = NULL;
            ec = error_code();
            size_t size = page_size();
            lb = alloc_raw_block(KEY_START, size, ec);
            if (lb) {
                head_ = new (lb->map_addr) Head(inst_id_, flag_);
                Mutex::scoped_lock lock(head_->mutex);
                head_->alloc_pos.addr = SharedPointer(KEY_START, sizeof(Head));
                head_->alloc_pos.end = head_->alloc_pos.addr + size;
                SharedPointer alloc_addr = head_->alloc_pos.addr;
                head_->alloc_pos.addr += sizeof(BlockItem);
                // 不通过alloc_addr获取地址，因为转换所需数据没有好，直接使用head_的紧接存储
                BlockItem * bi = new (head_ + 1) BlockItem;
                bi->addr = lb->map_addr;
                bi->key = head_->next_key;
                bi->size = size;
                bi->next = bi->last = 
                    SharedPointer(KEY_START, (char *)&head_->main_blocks - (char *)head_);;
                head_->main_blocks.next = alloc_addr;
                head_->main_blocks.last = alloc_addr;
                // 跟踪最后一个Block的key
                head_->main_blocks.key = bi->key;
                lb->block = bi;
                for (boost::uint32_t uid = 0; uid < SHARED_MEMORY_MAX_USER_ID; ++uid) {
                    head_->user_blocks[uid].next = head_->user_blocks[uid].last = 
                        SharedPointer(KEY_START, (char *)&head_->user_blocks[uid] - (char *)head_);
                }
                ++head_->next_key;
            }
            return lb;
        }

        SharedMemory::LocalBlock * SharedMemory::open_shm(
            error_code & ec)
        {
            LocalBlock * lb = NULL;
            ec = error_code();
            lb = open_raw_block(KEY_START, page_size(), ec);
            if (lb) {
                head_ = (Head *)lb->map_addr;
                assert(flag_ == head_->flag);
                lb->block = (BlockItem const *)(head_ + 1);
                Mutex::scoped_lock lock(head_->mutex);
                while (lb->block->addr == NULL) {
                    lock.unlock();
                    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
                    lock.lock();
                }
                instance_[head_->iid] = this;
                if (flag_ & exclusive) {
                    if (head_->attach[user_id_] == 0) {
                        head_->attach[user_id_] = (boost::uint32_t)time(NULL);
                    } else {
                        lock.unlock();
                        bool r = false;
                        free_raw_block(lb, r, ec);
                        ec = framework::system::logic_error::item_already_exist;
                    }
                }
            }
            return lb;
        }

        error_code SharedMemory::close(
            boost::system::error_code & ec)
        {
            Mutex::scoped_lock lock(head_->mutex);
            if (user_id_ < SHARED_MEMORY_MAX_USER_ID) {
                head_->attach[user_id_] = 0;
            }
            bool r = (user_id_ == SHARED_MEMORY_MAX_USER_ID); // remove user id
            for (size_t uid = 0; uid < SHARED_MEMORY_MAX_USER_ID; ++ uid) {
                while (LocalBlock * lb = local_blocks_[uid].last()) {
                    local_blocks_[uid].pop_back();
                    free_raw_block(lb, r, ec);
                    if (r) {
                        remove_block(head_->user_blocks[uid], *lb);
                    }
                    delete lb;
                }
            }
            delete [] local_blocks_;
            local_blocks_ = NULL;
            while (LocalBlock * lb = local_main_blocks_->last()) {
                local_main_blocks_->pop_back();
                if (lb->map_addr == head_) {
                    lock.unlock();
                }
                free_raw_block(lb, r, ec);
                if (r && lb->map_addr != head_) {
                    remove_block(head_->main_blocks, *lb);
                }
                delete lb;
            }
            delete local_main_blocks_;
            local_main_blocks_ = NULL;
            head_ = NULL;
            return ec;
        }

        void SharedMemory::open_for_remove(
            boost::uint32_t iid, 
            error_code & ec)
        {
            open_for_remove(iid, 0, ec);
        }

        void SharedMemory::open_for_remove(
            boost::uint32_t iid, 
            boost::uint32_t flag, 
            error_code & ec)
        {
            assert(!is_open());
            inst_id_ = iid;
            user_id_ = SHARED_MEMORY_MAX_USER_ID;
            flag_ = flag;

            assert((flag_ & 0xff) < sizeof(shared_memory_impls) / sizeof(shared_memory_impls[0]));
            impl_ = shared_memory_impls[flag_ & 0xff];

            LocalBlock * lb = open_raw_block(KEY_START, page_size(), ec);
            if (!lb) {
                return;
            } else {
                head_ = (Head *)lb->map_addr;
                lb->block = (BlockItem const *)(head_ + 1);
                local_main_blocks_ = new LocalBlockList;
                local_main_blocks_->push_back(lb);
                local_main_blocks_->next_to_open = (BlockItem const *)(head_ + 1);
                local_blocks_ = new LocalBlockList[SHARED_MEMORY_MAX_USER_ID];
                for (size_t uid = 0; uid < SHARED_MEMORY_MAX_USER_ID; ++uid) {
                    local_blocks_[uid].next_to_open = &head_->user_blocks[uid];
                }
            }
            check(head_->main_blocks, *local_main_blocks_);
            for (size_t uid = 0; uid < SHARED_MEMORY_MAX_USER_ID; ++uid) {
                check(head_->user_blocks[uid], local_blocks_[uid]);
            }
        }

        size_t SharedMemory::block_size(
            size_t size)
        {
            size_t tmp = head_->min_block_size;
            size = ((size - 1) / page_size() + 1) * page_size();
            head_->min_block_size = size;
            return tmp;
        }

        size_t SharedMemory::block_size() const
        {
            return head_->min_block_size;
        }

        void SharedMemory::insert_block(
            BlockItem & list, 
            LocalBlock & lb)
        {
            SharedPointer alloc_addr = head_->alloc_pos.addr;
            head_->alloc_pos.addr += sizeof(BlockItem);
            BlockItem * bi = new (alloc_addr.addr_of(this)) BlockItem;
            bi->addr = lb->map_addr;
            bi->key = head_->next_key;
            bi->size = lb.size;
            BlockItem * bi_tail = (BlockItem *)list.last.addr_of(this);
            bi->next = bi_tail->next; // bi_tail->next == list
            bi_tail->next = alloc_addr;
            bi->last = list.last;
            list.last = alloc_addr;
            // 跟踪最后一个Block的key
            list.key = bi->key;
            // attach to local block
            lb.block = bi;
        }

        void SharedMemory::remove_block(
            BlockItem & list, 
            LocalBlock & lb)
        {
            BlockItem * bi = (BlockItem *)lb.block;
            // detach from local block
            lb->block = NULL;

            BlockItem * next = (BlockItem *)bi->next.addr_of(this);
            BlockItem * last = (BlockItem *)bi->last.addr_of(this);
            last->next = bi->next;
            next->last = bi->last;
            bi->next = bi->last = SharedPointer();
            // 跟踪最后一个Block的key
            if (bi->key == list.key)
                list.key = last->key;
        }

        void * SharedMemory::alloc_block(
            size_t size)
        {
            if (!is_open()) {
                return NULL;
            }
            Mutex::scoped_lock lock(head_->mutex);
            error_code ec;
            // 保证有空间存储Block记录
            if (!valid_alloc(sizeof(BlockItem), ec))
                return NULL;
            // 释放刚刚申请的Block记录空间
            size = align_page(size);
            LocalBlock * lb = alloc_raw_block(head_->next_key, size, ec);
            if (lb == NULL) {
                return NULL;
            }
            BlockItem & head_blocks = 
                user_id_ == no_user_id ? head_->main_blocks : head_->user_blocks[user_id_];
            LocalBlockList & local_blocks = 
                user_id_ == no_user_id ? *local_main_blocks_ : local_blocks_[user_id_];
            check(head_blocks, local_blocks);
            insert_block(head_blocks, *lb);
            // insert_block需要head_->next_key，head_->next_key必须在insert_block之后调整
            ++head_->next_key;
            local_blocks.push_back(lb);
            local_blocks.next_to_open = static_cast<BlockItem const *>(lb->block);
            return lb->map_addr;
        }

        void SharedMemory::free_block(
            void * ptr, 
            size_t size)
        {
            if (!is_open()) {
                return;
            }
            Mutex::scoped_lock lock(head_->mutex);
            BlockItem & head_blocks = 
                user_id_ == no_user_id ? head_->main_blocks : head_->user_blocks[user_id_];
            LocalBlockList & local_blocks = 
                user_id_ == no_user_id ? *local_main_blocks_ : local_blocks_[user_id_];
            for (LocalBlock * lb = local_blocks.first(); lb; lb = local_blocks.next(lb)) {
                if (lb->block->addr == ptr) {
                    assert(lb->block->size == size);
                    local_blocks.erase(lb);
                    if (lb == local_blocks.last_use)
                        local_blocks.last_use = NULL;
                    error_code ec;
                    bool r = true;
                    free_raw_block(lb, r, ec);
                    if (r) {
                        remove_block(head_blocks, *lb);
                    }
                    delete lb;
                    return;
                }
            }
            assert(0);
        }

        void * SharedMemory::alloc_with_id(
            size_t id, 
            size_t size)
        {
            if (!is_open())
                return NULL;
            error_code ec;
            size = align_object(size);
            Mutex::scoped_lock lock(head_->mutex);
            IdObject * obj = &head_->objects;
            IdObject * obj1 = obj;
            while (obj->next != SharedPointer()) {
                obj = (IdObject *)obj->next.addr_of(this);
                if (obj->id == id) {
                    if (!(flag_ & alloc_get) || obj->size != size)
                        return NULL;
                    else
                        return obj + 1;
                }
                if (obj->id < id)
                    break;
                obj1 = obj;
            }
            if (!valid_alloc(sizeof(IdObject) + size, ec))
                return NULL;
            SharedPointer alloc_addr = head_->alloc_pos.addr;
            head_->alloc_pos.addr += sizeof(IdObject) + size;
            obj = new (alloc_addr.addr_of(this)) IdObject;
            obj->id = id;
            obj->size = size;
            obj->next = obj1->next;
            obj1->next = alloc_addr;
            return obj + 1;
        }

        void * SharedMemory::get_by_id(
            size_t id)
        {
            if (!is_open())
                return NULL;
            Mutex::scoped_lock lock(head_->mutex);
            IdObject * obj = &head_->objects;
            while (obj->next != SharedPointer()) {
                obj = (IdObject *)obj->next.addr_of(this);
                if (obj->id == id) {
                    return obj + 1;
                }
            }
            return NULL;
        }

        // don't check ec, check return value instead, if return value is not NULL, ec is undefined
        SharedMemory::LocalBlock * SharedMemory::alloc_raw_block(
            boost::uint32_t key, 
            boost::uint32_t size, 
            error_code & ec)
        {
            void * id = NULL;
            bool b = impl_->create(&id, inst_id_, key, size, ec);
            if (b == false) {
                LOG_F(Logger::kLevelError, "[alloc_raw_block] create failed (ec = %1%)" % ec.message());
                return NULL;
            }

            void * p = impl_->map(id, size, ec);
            if (NULL == p) {
                LOG_F(Logger::kLevelError, "[alloc_raw_block] map failed (ec = %1%)" % ec.message());
                error_code ec1;
                impl_->close(id, ec1);
                impl_->destory( inst_id_, key, ec1 );
                return NULL;
            }

            ec.clear();

            LocalBlock * lb = new LocalBlock;
            lb->map_id = id;
            lb->map_addr = p;
            lb->size = size;
            return lb;
        }

        // don't check ec, check return value instead, if return value is not NULL, ec is undefined
        SharedMemory::LocalBlock * SharedMemory::open_raw_block(
            boost::uint32_t key, 
            boost::uint32_t size, 
            error_code & ec)
        {
            void * id = NULL;
            bool b = impl_->open(&id, inst_id_, key, ec);
            if (b == false) {
                LOG_F(Logger::kLevelError, "[open_raw_block] open failed (ec = %1%)" % ec.message());
                return NULL;
            }

            void * p = impl_->map(id, size, ec);
            if (NULL == p) {
                LOG_F(Logger::kLevelError, "[open_raw_block] map failed (ec = %1%)" % ec.message());
                error_code ec1;
                impl_->close(id, ec);
                return NULL;
            }

            ec.clear();

            LocalBlock * lb = new LocalBlock;
            lb->map_id = id;
            lb->map_addr = p;
            lb->size = size;
            return lb;
        }

        void SharedMemory::free_raw_block(
            LocalBlock const * b, 
            bool & r, 
            error_code & ec)
        {
            int key = 0;
            if ( b->map_addr )
            {
                key = b->block->key;
                impl_->unmap(b->map_addr, b->size, ec);
                impl_->close(b->map_id, ec);
            }
            if ( r )
            {
                r = impl_->destory(inst_id_, key, ec);
            }
        }

        bool SharedMemory::valid_alloc(
            boost::uint32_t size, 
            boost::system::error_code & ec)
        {
            if (size + sizeof(BlockItem) > head_->alloc_pos.end - head_->alloc_pos.addr) {
                size = align_page(size + sizeof(BlockItem));
                LocalBlock * lb = alloc_raw_block(head_->next_key, size, ec);
                if (!lb)
                    return false;
                check(head_->main_blocks, *local_main_blocks_);
                insert_block(head_->main_blocks, *lb);
                local_main_blocks_->next_to_open = static_cast<BlockItem const *>(lb->block);
                // insert_block需要head_->next_key，head_->next_key必须在insert_block之后调整
                ++head_->next_key;
                local_main_blocks_->push_back(lb);
                head_->alloc_pos.addr = SharedPointer(lb->block->key);
                head_->alloc_pos.end = head_->alloc_pos.addr + size;
            }
            return true;
        }

        void * SharedMemory::alloc(
            size_t size, 
            error_code & ec)
        {
            size = align_object(size);
            Mutex::scoped_lock lock(head_->mutex);
            if (valid_alloc(size, ec)) {
                SharedPointer alloc_addr = head_->alloc_pos.addr;
                head_->alloc_pos.addr += size;
                return alloc_addr.addr_of(this);
            }
            return NULL;
        }

        void SharedMemory::check(
            BlockItem const & shared_list, 
            LocalBlockList & local_list)
        {
            if (!local_list.empty() && local_list.last()->block->key == shared_list.key)
                return;
            BlockItem const * bi = (BlockItem const *)local_list.next_to_open->next.addr_of(this);
            while (bi != &shared_list) {
                if (local_list.empty() || bi->key > local_list.last()->block->key) {
                    error_code ec;
                    LocalBlock * lb = open_raw_block(bi->key, bi->size, ec);
                    if (!lb)
                        break;
                    lb->block = bi;
                    LOG_F(Logger::kLevelDebug1, "[check] add block (iid = %1%, key = %2%)" % inst_id_ % lb->block->key);
                    local_list.push_back(lb);
                    local_list.next_to_open = bi;
                    bi = (BlockItem *)bi->next.addr_of(this);
                }
            }
        }

        void * SharedMemory::addr_ref_to_store(
            boost::uint32_t uid, 
            void * addr)
        {
            if (!is_open()) {
                return NULL;
            }
            if (addr == NULL)
                return NULL;
            if (uid == user_id_)
                return addr;
            LocalBlock const * lb = local_blocks_[uid].last_use;
            if (lb && lb->map_addr <= addr && addr < (char *)lb->map_addr + lb->block->size) {
                return (char *)lb->block->addr + ((char *)addr - (char *)(lb->map_addr));
            }
            for (LocalBlock const * lb = local_blocks_[uid].first(); lb; lb = local_blocks_[uid].next(lb)) {
                if (lb->map_addr <= addr && addr < (char *)lb->map_addr + lb->block->size) {
                    local_blocks_[uid].last_use = lb;
                    return (char *)lb->block->addr + ((char *)addr - (char *)(lb->map_addr));
                }
            }
            {
                Mutex::scoped_lock lock(head_->mutex);
                check(head_->user_blocks[uid], local_blocks_[uid]);
            }
            for (LocalBlock const * lb = local_blocks_[uid].first(); lb; lb = local_blocks_[uid].next(lb)) {
                if (lb->map_addr <= addr && addr < (char *)lb->map_addr + lb->block->size) {
                    local_blocks_[uid].last_use = lb;
                    return (char *)lb->block->addr + ((char *)addr - (char *)(lb->map_addr));
                }
            }
            LOG_F(Logger::kLevelError, "[addr_ref_to_store(1)] not found (uid = %1%, addr = %2%)" % uid % addr);
            return NULL;
        }

        void * SharedMemory::addr_store_to_ref(
            boost::uint32_t uid, 
            void * addr)
        {
            if (!is_open()) {
                return NULL;
            }
            if (addr == NULL)
                return NULL;
            if (uid == user_id_)
                return addr;
            LocalBlock const * lb = local_blocks_[uid].last_use;
            if (lb && lb->block->addr <= addr && addr < (char *)lb->block->addr + lb->block->size) {
                return (char *)lb->map_addr + ((char *)addr - (char *)lb->block->addr);
            }
            for (LocalBlock const * lb = local_blocks_[uid].first(); lb; lb = local_blocks_[uid].next(lb)) {
                if (lb->block->addr <= addr && addr < (char *)lb->block->addr + lb->block->size) {
                    local_blocks_[uid].last_use = lb;
                    return (char *)lb->map_addr + ((char *)addr - (char *)lb->block->addr);
                }
            }
            LOG_F(Logger::kLevelDebug, "[addr_store_to_ref(1)] need check (uid = %1%, addr = %2%)" % uid % addr);
            {
                Mutex::scoped_lock lock(head_->mutex);
                check(head_->user_blocks[uid], local_blocks_[uid]);
            }
            for (LocalBlock const * lb = local_blocks_[uid].first(); lb; lb = local_blocks_[uid].next(lb)) {
                if (lb->block->addr <= addr && addr < (char *)lb->block->addr + lb->block->size) {
                    local_blocks_[uid].last_use = lb;
                    return (char *)lb->map_addr + ((char *)addr - (char *)lb->block->addr);
                }
            }
            LOG_F(Logger::kLevelError, "[addr_store_to_ref(1)] not found (uid = %1%, addr = %2%)" % uid % addr);
            return NULL;
        }

        bool SharedMemory::addr_ref_to_store(
            void * addr, 
            boost::uint32_t & key, 
            size_t & off)
        {
            LocalBlock const * lb = local_main_blocks_->last_use;
            if (lb && lb->map_addr <= addr && addr < (char *)lb->map_addr + lb->block->size) {
                key = lb->block->key;
                off = (char *)addr - (char *)(lb->map_addr);
                return true;
            }
            for (LocalBlock const * lb = local_main_blocks_->first(); lb; lb = local_main_blocks_->next(lb)) {
                if (lb->map_addr <= addr && addr < (char *)lb->map_addr + lb->block->size) {
                    key = lb->block->key;
                    off = (char *)addr - (char *)(lb->map_addr);
                    local_main_blocks_->last_use = lb;
                    return true;
                }
            }
            LOG_F(Logger::kLevelError, "[addr_ref_to_store(2)] not found (addr = %1%)" % addr);
            return false;
        }

        bool SharedMemory::addr_store_to_ref(
            void *& addr, 
            boost::uint32_t key, 
            size_t off)
        {
            LocalBlock const * lb = local_main_blocks_->last_use;
            if (lb && lb->block->key == key) {
                addr = (char *)lb->map_addr + off;
                return true;
            }
            for (LocalBlock const * lb = local_main_blocks_->first(); lb; lb = local_main_blocks_->next(lb)) {
                if (lb->block->key == key) {
                    addr = (char *)lb->map_addr + off;
                    local_main_blocks_->last_use = lb;
                    return true;
                }
            }
            LOG_F(Logger::kLevelDebug, "[addr_store_to_ref(2)] need check (key = %1%, off = %2%)" % key % off);
            {
                Mutex::scoped_lock lock(head_->mutex);
                check(head_->main_blocks, *local_main_blocks_);
            }
            for (LocalBlock const * lb = local_main_blocks_->first(); lb; lb = local_main_blocks_->next(lb)) {
                if (lb->block->key == key) {
                    addr = (char *)lb->map_addr + off;
                    local_main_blocks_->last_use = lb;
                    return true;
                }
            }
            LOG_F(Logger::kLevelError, "[addr_store_to_ref(2)] not found (key = %1%, off = %2%)" % key % off);
            return false;
        }

        void * SharedMemory::get_by_id(
            boost::uint32_t iid, 
            size_t id)
        {
            if (iid < SHARED_MEMORY_MAX_INST_ID 
                && instance_[iid] != NULL)
                return instance_[iid]->get_by_id(id);
            return NULL;
        }

        void * SharedMemory::addr_ref_to_store(
            boost::uint32_t iid, 
            boost::uint32_t uid, 
            void * addr)
        {
            if (iid < SHARED_MEMORY_MAX_INST_ID 
                && instance_[iid] != NULL)
                return instance_[iid]->addr_ref_to_store(uid, addr);
            LOG_F(Logger::kLevelError, "[addr_ref_to_store(3)] inst not exist (iid = %1%)" % iid);
            return NULL;
        }

        void * SharedMemory::addr_store_to_ref(
            boost::uint32_t iid, 
            boost::uint32_t uid, 
            void * addr)
        {
            if (iid < SHARED_MEMORY_MAX_INST_ID 
                && instance_[iid] != NULL)
                return instance_[iid]->addr_store_to_ref(uid, addr);
            LOG_F(Logger::kLevelError, "[addr_store_to_ref(3)] inst not exist (iid = %1%)" % iid);
            return NULL;
        }

        bool SharedMemory::addr_ref_to_store(
            boost::uint32_t iid, 
            void * addr, 
            boost::uint32_t & key, 
            size_t & offset)
        {
            if (iid < SHARED_MEMORY_MAX_INST_ID 
                && instance_[iid] != NULL)
                return instance_[iid]->addr_ref_to_store(addr, key, offset);
            LOG_F(Logger::kLevelError, "[addr_ref_to_store(4)] inst not exist (iid = %1%)" % iid);
            return false;
        }

        bool SharedMemory::addr_store_to_ref(
            boost::uint32_t iid, 
            void *& addr, 
            boost::uint32_t key, 
            size_t offset)
        {
            if (iid < SHARED_MEMORY_MAX_INST_ID 
                && instance_[iid] != NULL)
                return instance_[iid]->addr_store_to_ref(addr, key, offset);
            LOG_F(Logger::kLevelError, "[addr_store_to_ref(4)] inst not exist (iid = %1%)" % iid);
            return false;
        }

    } // namespace memory
} // namespace framework
