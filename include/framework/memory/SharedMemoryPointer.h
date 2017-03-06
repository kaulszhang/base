// SharedMemoryPointer.h

#ifndef _FRAMEWORK_MEMORY_SHARED_MEMORY_POINTER_H_
#define _FRAMEWORK_MEMORY_SHARED_MEMORY_POINTER_H_

#include "framework/generic/Pointer.h"

namespace framework
{
    namespace memory
    {

        template <
            typename _Ty, 
            boost::uint32_t iid
        >
        class SharedMemoryPointer;

        namespace detail
        {
            struct key_of
            {
                template <
                    typename _Ty, 
                    boost::uint32_t iid
                >
                static boost::uint32_t invoke(
                    SharedMemoryPointer<_Ty, iid> const & pointer)
                {
                    return pointer.key_;
                }
            };

            struct off_of
            {
                template <
                    typename _Ty, 
                    boost::uint32_t iid
                >
                static size_t invoke(
                    SharedMemoryPointer<_Ty, iid> const & pointer)
                {
                    return pointer.off_;
                }
            };
        }

        template <
            typename _Ty, 
            boost::uint32_t iid
        >
        class SharedMemoryPointer
            : public framework::generic::Pointer<_Ty, SharedMemoryPointer<_Ty, iid> >
        {
        public:
            typedef _Ty value_type;

        public:
            SharedMemoryPointer()
                : key_(0)
                , off_(0)
            {
            }

            SharedMemoryPointer(
                _Ty * ptr)
                : key_(0)
                , off_(0)
            {
                if (ptr)
                    SharedMemory::addr_ref_to_store(iid, (void *)ptr, key_, off_);
            }

            template <typename _Ty1>
            SharedMemoryPointer(
                SharedMemoryPointer<_Ty1, iid> const & r)
                : key_(detail::key_of::invoke(r))
                , off_(detail::off_of::invoke(r))
            {
                (void)static_cast<_Ty *>((_Ty1 *)0);
            }

            SharedMemoryPointer & operator=(
                SharedMemoryPointer<_Ty, iid> const & r)
            {
                key_ = r.key_;
                off_ = r.off_;
                return *this;
            }

            template <typename _Ty1>
            SharedMemoryPointer & operator=(
                SharedMemoryPointer<_Ty1, iid> const & r)
            {
                *this = SharedMemoryPointer(r);
                return *this;
            }

            _Ty & operator*() const
            {
                void * ptr = NULL;
                if (key_)
                    SharedMemory::addr_store_to_ref(iid, ptr, key_, off_);
                return *static_cast<_Ty *>(ptr);
            }

            friend bool operator==(
                SharedMemoryPointer<_Ty, iid> const & l, 
                SharedMemoryPointer<_Ty, iid> const & r)
            {
                return l.key_ == r.key_ && l.off_ == r.off_;
            }

            friend bool operator!=(
                SharedMemoryPointer<_Ty, iid> const & l, 
                SharedMemoryPointer<_Ty, iid> const & r)
            {
                return !(l == r);
            }

            template <typename _Ty1>
            friend bool operator==(
                SharedMemoryPointer<_Ty, iid> const & l, 
                SharedMemoryPointer<_Ty1, iid> const & r)
            {
                return l == SharedMemoryPointer(r);
            }

            template <typename _Ty1>
            friend bool operator==(
                SharedMemoryPointer<_Ty1, iid> const & l, 
                SharedMemoryPointer<_Ty, iid> const & r);

            template <typename _Ty1>
            friend bool operator!=(
                SharedMemoryPointer<_Ty, iid> const & l, 
                SharedMemoryPointer<_Ty1, iid> const & r)
            {
                return !(l == r);
            }

            template <typename _Ty1>
            friend bool operator!=(
                SharedMemoryPointer<_Ty1, iid> const & l, 
                SharedMemoryPointer<_Ty, iid> const & r);

        private:
            friend struct detail::key_of;
            friend struct detail::off_of;

            boost::uint32_t key_;
            size_t off_;
        };


        template <
            typename _Ty, 
            boost::uint32_t iid
        >
        struct SharedMemoryPointerTraits
        {
            typedef _Ty value_type;

            typedef SharedMemoryPointer<_Ty, iid> pointer;

            typedef SharedMemoryPointer<_Ty const, iid> const_pointer;

            typedef value_type & reference;

            typedef const value_type & const_reference;

            typedef std::size_t size_type;

            typedef std::ptrdiff_t difference_type;

            template <typename _Ty1>
            struct rebind
            {
                typedef SharedMemoryPointerTraits<_Ty1, iid> type;
            };
        };

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_SHARED_MEMORY_POOL_H_
