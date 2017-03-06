// BigFirstPool.h

#ifndef _FRAMEWORK_MEMORY_BIG_FIRST_POOL_H_
#define _FRAMEWORK_MEMORY_BIG_FIRST_POOL_H_

#include "framework/container/Ordered.h"
#include "framework/container/OrderedUnidirList.h"
#include "framework/memory/MemoryPool.h"

namespace framework
{
    namespace memory
    {

        class BigFirstPool
            : public MemoryPool
        {
        public:
            BigFirstPool(
                BlockMemory & bm, 
                size_t capacity = (size_t)-1)
                : MemoryPool(bm, capacity)
            {
            }

            template <
                typename _Mm
            >
            BigFirstPool(
                _Mm const & mm, 
                size_t capacity = (size_t)-1)
                : MemoryPool(mm, capacity)
            {
            }

            BigFirstPool(
                BigFirstPool const & r, 
                AllocatorBind *)
                : MemoryPool(r)
            {
            }

            ~BigFirstPool()
            {
                clear();
            }

        private:
            struct Object;

            struct Block
                : framework::container::OrderedHook<Block>::type
            {
                size_t size;
                size_t size_used;

                Block(
                    size_t size)
                    : size(size)
                    , size_used(0)
                {
                }

                bool operator < (
                    Block const & r) const
                {
                    return this < &r;
                }

                bool full()
                {
                    return size_used == 0;
                }

                Object * first()
                {
                    return reinterpret_cast<Object *>(this + 1);
                }

                Object * last()
                {
                    return reinterpret_cast<Object *>(
                        reinterpret_cast<char *>(this) + size);
                }
            };

            struct ObjectHead
            {
                ObjectHead(
                    Block * blk, 
                    size_t size)
                    : blk(blk)
                    , size(size)
                {
                }

                Block * blk;
                size_t size; // 整个内存块的大小
            };

            struct Object
                : MemoryPool::Object<ObjectHead>
                , framework::container::OrderedHook<Object>::type
            {
                Object(
                    Block * blk, 
                    size_t size)
                    : MemoryPool::Object<ObjectHead>(blk, size)
                {
                }

                Object * successive()
                {
                    return reinterpret_cast<Object *>(
                        reinterpret_cast<char *>(this) + size);
                }

                void * data()
                {
                    return (char *)(this) + size_head;
                }

                void * to_data()
                {
                    assert(!is_used());
                    use();
                    return (char *)(this) + size_head;
                }

                void from_data()
                {
                    assert(is_used());
                    static_cast<framework::container::OrderedHook<Object>::type &>(*this)
                        = framework::container::OrderedHook<Object>::type();
                    unusemark();
                }

                static Object * from_data(
                    void * ptr)
                {
                    Object * obj = (Object *)((char *)ptr - size_head);
                    obj->from_data();
                    return obj;
                }

                static Object const * from_data(
                    void const * ptr)
                {
                    return (Object const *)((char const *)ptr - size_head);
                }

                void use()
                {
                    blk = (Block *)((unsigned long)blk | 1);
                }

                void unuse()
                {
                    blk = (Block *)((unsigned long)blk & -2);
                }

                bool is_used()
                {
                    return ((unsigned long)blk & 1) != 0;
                }

                void mark()
                {
                    blk = (Block *)((unsigned long)blk | 2);
                }

                void unmark()
                {
                    blk = (Block *)((unsigned long)blk & -3);
                }

                bool is_marked()
                {
                    return ((unsigned long)blk & 2) != 0;
                }

                void unusemark()
                {
                    blk = (Block *)((unsigned long)blk & -4);
                }

                bool operator < (
                    Object const & r) const
                {
                    return size > r.size;
                }

                BOOST_STATIC_CONSTANT(size_t, size_head = 
                    MemoryPage::static_align_object<sizeof(MemoryPool::Object<ObjectHead>)>::value);
            };

        public:
            void * alloc(
                size_t size)
            {
                size = MemoryPage::align_object(size);
                size += size_head;
                if (size < sizeof(Object)) {
                    size = sizeof(Object);
                }
                if (free_objs_.empty() || free_objs_.begin()->size < size) {
                    size_t block_size = MemoryPage::align_page(size + sizeof(Block));
                    if (consumption() + block_size > capacity())
                        return NULL;
                    void * ptr = MemoryPool::alloc_block(block_size);
                    if (ptr == NULL)
                        return NULL;
                    attach_block(ptr, block_size);
                    check();
                }
                Object * obj = free_objs_.first();
                Block * blk = obj->blk;
                free_objs_.erase(obj);
                size_used_ += obj->size;
                //check();
                size_used_ -= obj->size;
                assert(obj->size >= size);
                // 如果剩余的大小还能构成一个Object，创建一个新Object
                if (obj->size - size > sizeof(Object)) {
                    obj->size -= size;
                    assert((int)obj->size > (int)sizeof(Object));
                    free_objs_.insert(obj);
                    obj = obj->successive();
                    new (obj) Object(blk, size);
                }
                size_used_ += obj->size;
                blk->size_used += obj->size;
                MemoryPool::alloc_object(obj);
                void * ptr = obj->to_data();
                check();
                return ptr;
            }

            void free(
                void * ptr)
            {
                // ignore NULL
                if (ptr == NULL)
                    return;

                Object * obj = Object::from_data(ptr);

                assert(!obj->is_used());

                Block * blk = obj->blk;
                blk->size_used -= obj->size;

                //check();

                size_used_ -= obj->size;
                MemoryPool::free_object(obj);

                Object * obj1 = blk->first();
                for (; obj1 < obj; ) {
                    if (obj1->successive() == obj) {
                        if (!obj1->is_used()) {
                            free_objs_.erase(obj1);
                            obj1->size += obj->size;
                            obj->~Object();
                            obj = obj1;
                        }
                        break;
                    }
                    obj1 = obj1->successive();
                }

                obj1 = obj->successive();
                if (obj1 < blk->last() && !obj1->is_used()) {
                    free_objs_.erase(obj1);
                    obj->size += obj1->size;
                    obj1->~Object();
                }

                free_objs_.insert(obj);
                    
                check();

                if (blk->full() && size_used_ + blk->size + MemoryPage::page_size() < consumption()) {
                    detach_block(blk);
                    MemoryPool::free_block(blk, blk->size);
                }

                check();
            }

            size_t query_size(
                void const * ptr)
            {
                Object const * obj = Object::from_data(ptr);
                return obj->size - sizeof(Object);
            }

            void mark()
            {
                for (Block * blk = blocks_.first(); blk; blk = blocks_.next(blk)) {
                    if (!blk->full()) {
                        Object * obj_beg = blk->first();
                        Object * obj_end = blk->last();
                        for (Object * obj = obj_beg; obj < obj_end; ) {
                            if (obj->is_used()) {
                                obj->mark();
                            }
                            obj = obj->successive();
                        }
                    }
                }
            }

            void leak()
            {
                for (Block * blk = blocks_.first(); blk; blk = blocks_.next(blk)) {
                    if (!blk->full()) {
                        Object * obj_beg = blk->first();
                        Object * obj_end = blk->last();
                        for (Object * obj = obj_beg; obj < obj_end; ) {
                            if (obj->is_used() && !obj->is_marked()) {
                                leak_object(obj, obj->data(), obj->size - size_head);
                            }
                            obj = obj->successive();
                        }
                    }
                }
            }

        private:
            void attach_block(
                void * ptr, 
                size_t size)
            {
                Block * blk = new (ptr) Block(size);
                blocks_.insert(blk);
                size_used_ += sizeof(Block);
                Object * obj = new (blk + 1) Object(blk, size - sizeof(Block));
                free_objs_.insert(free_objs_.begin(), obj);
            }

            void detach_block(
                Block * blk)
            {
                Object * obj = (Object *)(blk + 1);
                assert(obj->blk == blk && obj->size == blk->size - sizeof(Block));
                free_objs_.erase(obj);
                obj->~Object();
                blocks_.erase(blk);
                blk->~Block();
                size_used_ -= sizeof(Block);
            }

            void clear()
            {
                while (Block * blk = blocks_.first()) {
                    if (!blk->full()) {
                        Object * obj_beg = blk->first();
                        Object * obj_end = blk->last();
                        for (Object * obj = obj_beg; obj < obj_end; ) {
                            if (obj->is_used()) {
                                leak_object(obj, obj->data(), obj->size - size_head);
                                obj->from_data();
                                blk->size_used -= obj->size;
                            } else {
                                free_objs_.erase(obj);
                            }
                            assert(obj->blk == blk);
                            Object * obj1 = obj;
                            obj = obj->successive();
                            if (obj1 != obj_beg) {
                                obj1->~Object();
                            }
                        }
                        // for detach_block to erase
                        free_objs_.insert(new (obj_beg) Object(blk, blk->size - sizeof(Block)));
                    }
                    detach_block(blk);
                    free_block(blk, blk->size);
                }
                assert(free_objs_.empty());
            }

            void check()
            {
#if (defined DEBUG && 0)
                size_t size = size_used_;
                size_t size2 = 0;
                for (Object * obj = free_objs_.first(); obj; obj = free_objs_.next(obj)) {
                    assert(obj->!is_used());
                    assert((int)obj->size >= (int)sizeof(Object));
                    size += obj->size;
                    //std::cout << obj << " " << obj->size << std::endl;
                }
                //std::cout << std::endl;
                for (Block * blk = blocks_.first(); blk; blk = blocks_.next(blk)) {
                    Object * obj_beg = blk->first();
                    Object * obj_end = blk->last();
                    size_t size3 = sizeof(Block);
                    size_t size4 = sizeof(Block);
                    for (Object * obj = obj_beg; obj < obj_end; obj = obj->successive()) {
                        if (obj->!is_used()) {
                            assert(free_objs_.exist(obj));
                            size3 += obj->size;
                        }
                        size4 += obj->size;
                    }
                    assert(size3 == blk->size - blk->size_used);
                    assert(size4 == blk->size);
                    size2 += blk->size;
                }
                assert(size == size2);
#endif
            }

        private:
            typedef framework::container::Ordered<
                Object, 
                framework::container::identity<Object>, 
                std::less<Object>, 
                framework::container::ordered_non_unique_tag
            > FreeObjectSet;

            typedef framework::container::Ordered<
                Block
            > BlockSet;

        private:
            BOOST_STATIC_CONSTANT(size_t, size_head = Object::size_head);

        private:
            size_t size_used_;
            BlockSet blocks_;
            FreeObjectSet free_objs_;
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_BIG_FIRST_POOL_H_
