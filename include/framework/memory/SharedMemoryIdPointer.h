// SharedMemoryIdPointer.h

#ifndef _FRAMEWORK_MEMORY_SHARED_MEMORY_ID_POINTER_H_
#define _FRAMEWORK_MEMORY_SHARED_MEMORY_ID_POINTER_H_

#include "framework/generic/Pointer.h"

namespace framework
{
    namespace memory
    {

        template <
            typename _Ty, 
            boost::uint32_t iid
        >
        class SharedMemoryIdPointer;

        template <
            typename _Ty, 
            boost::uint32_t iid
        >
        class SharedMemoryIdPointer
            : public framework::generic::Pointer<_Ty, SharedMemoryIdPointer<_Ty, iid> >
        {
        public:
            typedef _Ty value_type;

        public:
            SharedMemoryIdPointer()
                : ptr_(NULL)
                , id_(0)
            {
            }

            SharedMemoryIdPointer(
                size_t id)
                : ptr_(NULL)
                , id_(id)
            {
            }

            _Ty & operator*() const
            {
                if (ptr_)
                    return *ptr_;
                return *static_cast<_Ty *>(
                    SharedMemory::get_by_id(iid, id_));
            }

            friend bool operator== (
                SharedMemoryIdPointer<_Ty, iid> const & l, 
                SharedMemoryIdPointer<_Ty, iid> const & r)
            {
                return l.id_ == r.id_;
            }

            friend bool operator!= (
                SharedMemoryIdPointer<_Ty, iid> const & l, 
                SharedMemoryIdPointer<_Ty, iid> const & r)
            {
                return l.id_ != r.id_;
            }

            template <typename _Ty1>
            friend bool operator== (
                SharedMemoryIdPointer<_Ty, iid> const & l, 
                SharedMemoryIdPointer<_Ty1, iid> const & r)
            {
                (void)((_Ty1 *)0 == (_Ty *)0);
                return l.id_ == r.id_;
            }

            template <typename _Ty1>
            friend bool operator== (
                SharedMemoryIdPointer<_Ty1, iid> const & l, 
                SharedMemoryIdPointer<_Ty, iid> const & r);

            template <typename _Ty1>
            friend bool operator!= (
                SharedMemoryIdPointer<_Ty, iid> const & l, 
                SharedMemoryIdPointer<_Ty1, iid> const & r)
            {
                return !(l == r);
            }

            template <typename _Ty1>
            friend bool operator!= (
                SharedMemoryIdPointer<_Ty1, iid> const & l, 
                SharedMemoryIdPointer<_Ty, iid> const & r);

        private:
            _Ty * ptr_;
            size_t id_;
        };

        template <
            typename _Ty, 
            boost::uint32_t iid
        >
        struct SharedMemoryIdPointerTraits
        {
            typedef _Ty value_type;

            typedef SharedMemoryIdPointer<_Ty, iid> pointer;

            typedef SharedMemoryIdPointer<_Ty const, iid> const_pointer;

            typedef value_type & reference;

            typedef const value_type & const_reference;

            typedef std::size_t size_type;

            typedef std::ptrdiff_t difference_type;

            template <typename _Ty1>
            struct rebind
            {
                typedef SharedMemoryIdPointerTraits<_Ty1, iid> type;
            };
        };

    } // namespace memory
} // namespace framework

inline void * operator new (
                            size_t size, 
                            framework::memory::SharedMemory & m, 
                            size_t id) throw ()
{
    return m.alloc_with_id(id, size);
}

inline void operator delete (
                             void * ptr, 
                             framework::memory::SharedMemory & m, 
                             size_t id) throw ()
{
    assert(0);
}

inline void * operator new[] (
                              size_t size, 
                              framework::memory::SharedMemory & m, 
                              size_t id) throw ()
{
    return m.alloc_with_id(id, size);
}

inline void operator delete[] (
                               void * ptr, 
                               framework::memory::SharedMemory const *, 
                               size_t id) throw ()
{
    assert(0);
}

#define SHARED_NEW_ID(m, id) new (m, id)

#endif // _FRAMEWORK_MEMORY_SHARED_MEMORY_ID_POINTER_H_
