// ObjectAllocator.h

// 适应标准库的allocator，默认使用ObjectMemoryPool
// 该allocator使用了静态成员变量，由于全局变量的初始化顺序不确定的问题，
// 使用该allocator的标准库容器不能在全局构造（不能定义全局变量或者类静态成员变量，但可以是指针）

#ifndef _FRAMEWORK_MEMORY_OBJECT_ALLOCATOR_H_
#define _FRAMEWORK_MEMORY_OBJECT_ALLOCATOR_H_

#include "framework/memory/MemoryPool.h"
#include "framework/memory/PrivateMemory.h"
#include "framework/memory/BigFixedPool.h"
#include "framework/generic/NativePointer.h"

namespace framework
{
    namespace memory
    {

        template <
            typename _Ty, 
            typename _Pl = framework::memory::BigFixedPool, 
            typename _Pt = framework::generic::NativePointerTraits<_Ty>
        >
        class ObjectAllocator
        {
        public:
            typedef _Ty value_type;

            typedef typename _Pt::pointer pointer;

            typedef typename _Pt::const_pointer const_pointer;

            typedef typename _Pt::reference reference;

            typedef typename _Pt::const_reference const_reference;

            typedef typename _Pt::size_type size_type;

            typedef typename _Pt::difference_type difference_type;

        public:
            ObjectAllocator()
                : pool_(PrivateMemory())
            {
            }

            ObjectAllocator(
                _Pl const & pool)
                : pool_(pool)
            {
            }

            ObjectAllocator(
                ObjectAllocator const & r)
                : pool_(r.pool_)
            {
            }

            ~ObjectAllocator()
            {
            }

        public:
            pointer address(reference x) const
            {
                return &x;
            }

            const_pointer address(const_reference x) const
            {
                return &x;
            }

        public:
            pointer allocate(
                size_type n, 
                const_pointer = 0) throw(std::bad_alloc)
            {
                void * p = pool_.alloc(n * sizeof(_Ty));
                if (!p)
                    throw std::bad_alloc();
                return pointer(static_cast<value_type *>(p));
            }

            void deallocate(
                pointer p, 
                size_type n)
            {
                pool_.free(p, n * sizeof(_Ty));
            }

            void deallocate(
                void * p, 
                size_type)
            {
                pool_.free(p);
            }

            size_type max_size() const
            {
                return static_cast<size_type>(-1) / sizeof(value_type);
            }

        public:
            bool operator == (
                const ObjectAllocator &) const
            {
                return true;
            }

            bool operator != (
                const ObjectAllocator &) const
            {
                return false;
            }

        public:
            void construct(
                pointer p, 
                const value_type& x)
            {
                new(p) value_type(x);
            }

            void destroy(
                pointer p)
            {
                (&*p)->~_Ty();
            }

        public:
            _Pl const & pool() const
            {
                return pool_;
            }

            template <typename _Ty1>
            ObjectAllocator(
                ObjectAllocator<
                    _Ty1, 
                    _Pl, 
                    typename _Pt::template rebind<_Ty1>::type
                > const & r)
                : pool_(r.pool(), (framework::memory::AllocatorBind *)NULL)
            {
            }
			
            template <class _Ty1>
            struct rebind
            {
                typedef typename _Pt::template rebind<_Ty1>::type _Pt1;
                typedef ObjectAllocator<_Ty1, _Pl, _Pt1> other;
            };

        private:
            void operator = (
                const ObjectAllocator &);

        private:
            _Pl pool_;
        };


        template<
            typename _Pl, 
            typename _Pt
        >
        class ObjectAllocator<void, _Pl, _Pt>
        {
        public:
            typedef void value_type;

            typedef void * pointer;

            typedef const void * const_pointer;

        public:
            template <class _Ty1>
            struct rebind
            {
                typedef ObjectAllocator<
                    _Ty1, 
                    _Pl, 
                    typename _Pt::template rebind<_Ty1>::type> other;
            };
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_OBJECT_ALLOCATOR_H_
