// FixedPool.h

#ifndef _FRAMEWORK_MEMORY_FIXED_POOL_H_
#define _FRAMEWORK_MEMORY_FIXED_POOL_H_

#include "framework/container/List.h"
#include "framework/container/UnidirList.h"
#include "framework/container/OrderedUnidirList.h"
#include "framework/memory/MemoryPool.h"

#include <boost/type_traits/is_empty.hpp>

namespace framework
{
    namespace memory
    {

        template <
            typename Object
        >
        struct FixedBlock
            : public framework::container::ListHook<FixedBlock<Object> >::type
        {
            typedef framework::container::UnidirList<
                Object 
            > ObjectList;

            FixedBlock(
                size_t size, 
                size_t size_object)
                : size_block_(size)
                , size_object_(size_object)
                , num_used_(0)
            {
                char * beg = (char *)this + head_size();
                char * end = (char *)this + size_block_ - size_object_;
                for (; beg <= end; beg += size_object_) {
                    Object * obj = new (beg) Object(this);
                    free_objs_.push_front(obj);
                }
            }

            ~FixedBlock()
            {
                clear();
            }

            Object * pop()
            {
                Object * obj = free_objs_.first();
                free_objs_.pop_front();
                ++num_used_;
                return obj;
            }

            void push(Object * obj)
            {
                free_objs_.push_front(obj);
                --num_used_;
            }

            void clear()
            {
                while (Object * obj = free_objs_.first()) {
                    free_objs_.pop_front();
                    obj->~Object();
                }
            }

            bool empty() const
            {
                return free_objs_.empty();
            }

            bool full() const
            {
                return num_used_ == 0;
            }

            size_t size_object() const
            {
                return size_object_;
            }

            size_t size_block() const
            {
                return size_block_;
            }

            size_t capacity() const
            {
                return (size_block_ - head_size()) / size_object_;
            }

            struct object_key
            {
                typedef Object const * result_type;

                result_type operator()(
                    Object const & o)
                {
                    return &o;
                }
            };

            struct greater_addr
            {
                bool operator()(
                    Object const * l, 
                    Object const * r)
                {
                    return l > r;
                }
            };

            template <
                typename Pool
            >
            void leak(
                Pool pool)
            {
                typedef framework::container::OrderedUnidirList<
                    Object, 
                    object_key, 
                    greater_addr
                > OrderedObjectList;

                OrderedObjectList lst;
                while (Object * obj = free_objs_.first()) {
                    free_objs_.pop_front();
                    lst.push(obj);
                }
                while (Object * obj = lst.first()) {
                    lst.pop();
                    assert(free_objs_.empty() || obj < free_objs_.first());
                    free_objs_.push_front(obj);
                }

                char * beg = (char * )(this + 1);
                char * end = (char * )this + size_block_ - size_object_;
                Object * obj = free_objs_.first();
                for (; beg <= end && obj; ) {
                    if (beg < (char *)obj) {
                        pool((Object *)beg);
                        beg += size_object_;
                    } else if (beg > (char *)obj) {
                        obj = free_objs_.next(obj);
                    } else {
                        beg += size_object_;
                        obj = free_objs_.next(obj);
                    }
                }
                while (beg <= end) {
                    pool((Object *)beg);
                    beg += size_object_;
                }
            }

        private:
            static size_t head_size()
            {
                return MemoryPage::static_align_object<sizeof(FixedBlock)>::value;
            }

        private:
            size_t size_block_;
            size_t size_object_;
            size_t num_used_;
            ObjectList free_objs_;
        };

        template <
            typename ObjectHead, 
            bool head_empty = boost::is_empty<ObjectHead>::value
        >
        struct FixedObject
            : ObjectHead
            , framework::container::UnidirListHook<FixedObject<ObjectHead, head_empty> >::type
        {
            FixedObject(
                FixedBlock<FixedObject> * blk)
                : ObjectHead(blk)
            {
            }

            FixedBlock<FixedObject> * block() const
            {
                return (FixedBlock<FixedObject> *)ObjectHead::block();
            }

            ObjectHead * head()
            {
                return this;
            }

            void * data() const
            {
                return (char *)this + size_head;
            }

            void * to_data()
            {
                return (char *)this + size_head;
            }

            void from_data()
            {
            }

            static FixedObject * from_data(
                void * ptr)
            {
                return (FixedObject *)((char *)ptr - size_head);
            }

            static FixedObject const * from_data(
                void const * ptr)
            {
                return (FixedObject const *)((char const *)ptr - size_head);
            }

            BOOST_STATIC_CONSTANT(size_t, size_head = 
                MemoryPage::static_align_object<sizeof(ObjectHead)>::value);
        };

        template <
            typename ObjectHead
        >
        struct FixedObject<ObjectHead, true>
            : framework::container::UnidirListHook<FixedObject<ObjectHead, true> >::type
        {
            FixedObject(
                FixedBlock<FixedObject> * blk)
            {
            }

            size_t block_size(
                size_t size_block_head, 
                size_t size_object) const
            {
                return ObjectHead().block_size(size_block_head, size_object);
            }

            FixedBlock<FixedObject> * block() const
            {
                return (FixedBlock<FixedObject> *)((ObjectHead *)this)->block();
            }

            ObjectHead * head()
            {
                return (ObjectHead *)this;
            }

            void * data() const
            {
                return (void *)this;
            }

            void * to_data()
            {
                return this;
            }

            void from_data()
            {
            }

            static FixedObject * from_data(
                void * ptr)
            {
                return (FixedObject *)(ptr);
            }

            static FixedObject const * from_data(
                void const * ptr)
            {
                return (FixedObject const *)(ptr);
            }

            BOOST_STATIC_CONSTANT(size_t, size_head = 0);
        };

        template <
            typename _FB
        >
        class FixedPool
            : public MemoryPool
            , _FB
        {
        public:
            FixedPool(
                BlockMemory & bm, 
                size_t capacity = -1, 
                size_t size_object = 0)
                : MemoryPool(bm, capacity)
                , size_object_(0)
            {
                if (size_object)
                    change_size(size_object);
            }

            template <
                typename _Mm
            >
            FixedPool(
                _Mm const & mm, 
                size_t capacity = -1, 
                size_t size_object = 0)
                : MemoryPool(mm, capacity)
                , size_object_(0)
            {
                if (size_object)
                    change_size(size_object);
            }

            FixedPool(
                FixedPool const & r)
                : MemoryPool(r)
                , size_object_(r.size_object_)
                , object_per_block_(r.object_per_block_)
                , max_object_(r.max_object_)
            {
            }

            FixedPool(
                FixedPool const & r, 
                AllocatorBind *)
                : MemoryPool(r)
                , size_object_(0)
                , object_per_block_(0)
                , max_object_(0)
            {
            }

            ~FixedPool()
            {
                clear();
            }

        public:
            void * alloc()
            {
                if (free_blocks_.empty()) {
                    Block * blk = (Block *)alloc_block(size_block_);
                    if (blk) {
                        attach_block(blk);
                    } else {
                        return NULL;
                    }
                }

                Block * blk = free_blocks_.first();
                Object * obj = blk->pop();
                alloc_object(obj->head());

                if (blk->empty()) {
                    free_blocks_.erase(blk);
                    empty_blocks_.insert(blk);
                }

                return obj->to_data();
            }

            void * alloc(
                size_t size)
            {
                if (size <= size_object_ || change_size(size)) {
                    return alloc();
                } else {
                    assert(0);
                    return NULL;
                }
            }

            void free(
                void * ptr)
            {
                if (ptr == NULL)
                    return;

                Object * obj = Object::from_data(ptr);

                Block * blk = obj->block();
                bool empty = blk->empty();

                free_object(obj->head());
                blk->push(obj);

                if (empty
                    && blk->size_object() == size_object_ + size_head) {
                        empty_blocks_.erase(blk);
                        free_blocks_.insert(free_blocks_.end(), blk);
                }
                if (blk->full() 
                    && (blk != free_blocks_.first() || blk->size_object() != size_object_ + size_head)) {
                        detach_block(blk);
                        free_block(blk, blk->size_block());
                }
            }

            void free(
                void * ptr, 
                size_t size)
            {
                assert(size <= size_object_);
                free(ptr);
            }

        public:
            size_t size_object() const
            {
                return size_object_;
            }

            size_t size_block() const
            {
                return size_block_;
            }

            size_t object_per_block() const
            {
                return object_per_block_;
            }

            size_t max_object() const
            {
                return max_object_;
            }

            size_t left_object() const    // 还能申请多少个
            {
                return max_object() > num_object() ? 
                    max_object() - num_object() : 0;
            }

        private:
            typedef MemoryPool::Object<typename _FB::Object> ObjectHead;

            typedef FixedObject<ObjectHead> Object;

            typedef FixedBlock<Object> Block;

        private:
            bool change_size(
                size_t size_object)
            {
                size_object = MemoryPage::align_object(size_object);
                if (size_object + size_head < sizeof(Object))
                    size_object = MemoryPage::align_object(sizeof(Object) - size_head);
                size_t size_object_all = size_object + size_head;
                size_t size_block = _FB::change_size(sizeof(Block), size_object_, size_object_all);
                if (size_block) {
                    size_object_ = size_object;
                    size_block_ = size_block;
                    object_per_block_ = (size_block_ - sizeof(Block)) / size_object_all;
                    max_object_ = capacity() / size_block_ * object_per_block_;
                    assert(max_object_);
                    while (Block * blk = free_blocks_.first()) {
                        free_blocks_.erase(blk);
                        empty_blocks_.insert(blk);
                    }
                }
                return size_block > 0;
            }

            void attach_block(
                void * ptr)
            {
                Block * blk = new (ptr) Block(size_block_, size_object_ + size_head);
                free_blocks_.insert(free_blocks_.begin(), blk);
            }

            void detach_block(
                Block * blk)
            {
                // 可能blk在empty_blocks_里面，不过没有关系
                free_blocks_.erase(blk);
                blk->~Block();
            }

            struct LeakCall
            {
                LeakCall(
                    FixedPool & pool, 
                    Block * blk)
                    : pool_(pool)
                    , blk_(blk)
                {
                }

                void operator()(
                    Object * obj)
                {
                    pool_.leak_object(blk_, obj);
                }

            private:
                FixedPool & pool_;
                Block * blk_;
            };

            friend struct LeakCall;

            void leak_object(
                Block * blk, 
                Object * obj)
            {
                obj->from_data();
                MemoryPool::leak_object(obj->head(), obj->data(), blk->size_object() - size_head);
            }

            void clear()
            {
                while (Block * blk = empty_blocks_.first()) {
                    empty_blocks_.erase(blk);
                    free_blocks_.insert(blk);
                }
                while (Block * blk = free_blocks_.first()) {
                    if (!blk->full()) {
                        blk->leak(LeakCall(*this, blk));
                    }
                    detach_block(blk);
                    free_block(blk, blk->size_block());
                }
            }

        private:
            size_t size_object_; // 对象大小，字节数
            size_t size_block_;
            size_t object_per_block_;
            size_t max_object_; // 最多能申请多少个
            
            BOOST_STATIC_CONSTANT(size_t, size_head = Object::size_head);

            typedef framework::container::List<
                Block
            > BlockList;

            BlockList free_blocks_; // 有一到多个空闲Object的Block
            BlockList empty_blocks_; // 没有空闲Object的Block
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_FIXED_POOL_H_
