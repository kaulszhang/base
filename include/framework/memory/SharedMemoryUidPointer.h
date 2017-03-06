// SharedMemoryUidPointer.h

#ifndef _FRAMEWORK_MEMORY_SHARED_MEMORY_UID_POINTER_H_
#define _FRAMEWORK_MEMORY_SHARED_MEMORY_UID_POINTER_H_

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
            boost::uint32_t iid, 
            boost::uint32_t uid // 创建者ID
        >
        class SharedMemoryUidPointer;

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

        namespace detail
        {
            struct addr_of
            {
                template <
                    typename _Ty, 
                    boost::uint32_t iid, 
                    boost::uint32_t uid // 创建者ID
                >
                static _Ty * invoke(SharedMemoryUidPointer<_Ty, iid, uid> const & pointer)
                {
                    return pointer.addr_;
                }
            };
        }

        template <
            typename _Ty, 
            boost::uint32_t iid, 
            boost::uint32_t uid // 创建者ID
        >
        class SharedMemoryUidPointer
            : public framework::generic::Pointer<_Ty, SharedMemoryUidPointer<_Ty, iid, uid> >
        {
        public:
            typedef _Ty value_type;

        public:
            SharedMemoryUidPointer()
                : addr_(NULL)
            {
            }

            SharedMemoryUidPointer(
                _Ty * ptr)
            {
                addr_ = reinterpret_cast<_Ty *>(const_cast<void *>(
                    static_cast<void const *>(
                    SharedMemory::addr_ref_to_store(iid, uid, 
                    const_cast<void *>(static_cast<void const *>(ptr))))));
            }

            template <typename _Ty1>
            SharedMemoryUidPointer(
                SharedMemoryUidPointer<_Ty1, iid, uid> const & r)
                : addr_(detail::addr_of::invoke(r))
            {
            }

            SharedMemoryUidPointer & operator= (
                SharedMemoryUidPointer<_Ty, iid, uid> const & r)
            {
                addr_ = r.addr_;
                return *this;
            }

            template <typename _Ty1>
            SharedMemoryUidPointer & operator=(
                SharedMemoryUidPointer<_Ty1, iid, uid> const & r)
            {
                addr_ = detail::addr_of::invoke(r);
                return *this;
            }

            _Ty & operator*() const
            {
                return *static_cast<_Ty *>(
                    SharedMemory::addr_store_to_ref(iid, uid, 
                    const_cast<void *>(static_cast<const void *>(addr_))));
            }

            friend bool operator==(
                SharedMemoryUidPointer<_Ty, iid, uid> const & l, 
                SharedMemoryUidPointer<_Ty, iid, uid> const & r)
            {
                return l.addr_ == r.addr_;
            }

            friend bool operator!=(
                SharedMemoryUidPointer<_Ty, iid, uid> const & l, 
                SharedMemoryUidPointer<_Ty, iid, uid> const & r)
            {
                return l.addr_ != r.addr_;
            }

            template <typename _Ty1>
            friend bool operator==(
                SharedMemoryUidPointer<_Ty, iid, uid> const & l, 
                SharedMemoryUidPointer<_Ty1, iid, uid> const & r)
            {
                return l.addr_ == detail::addr_of::invoke(r);
            }

            template <typename _Ty1>
            friend bool operator== (
                SharedMemoryUidPointer<_Ty1, iid, uid> const & l, 
                SharedMemoryUidPointer<_Ty, iid, uid> const & r);

            template <typename _Ty1>
            friend bool operator!= (
                SharedMemoryUidPointer<_Ty, iid, uid> const & l, 
                SharedMemoryUidPointer<_Ty1, iid, uid> const & r)
            {
                return !(l == r);
            }

            template <typename _Ty1>
            friend bool operator!= (
                SharedMemoryUidPointer<_Ty1, iid, uid> const & l, 
                SharedMemoryUidPointer<_Ty, iid, uid> const & r);

        private:
            friend struct detail::addr_of;

            _Ty * addr_;
        };


        template <
            typename _Ty, 
            boost::uint32_t iid, 
            boost::uint32_t uid // 创建者ID
        >
        struct SharedMemoryUidPointerTraits
        {
            typedef _Ty value_type;

            typedef SharedMemoryUidPointer<_Ty, iid, uid> pointer;

            typedef SharedMemoryUidPointer<_Ty const, iid, uid> const_pointer;

            typedef value_type & reference;

            typedef const value_type & const_reference;

            typedef std::size_t size_type;

            typedef std::ptrdiff_t difference_type;

            template <typename _Ty1>
            struct rebind
            {
                typedef SharedMemoryUidPointerTraits<_Ty1, iid, uid> type;
            };
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_SHARED_MEMORY_UID_POINTER_H_
