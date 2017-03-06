// RawPointerTraits.h

#ifndef _FRAMEWORK_GENERIC_RAW_POINTER_H_
#define _FRAMEWORK_GENERIC_RAW_POINTER_H_

#include "framework/generic/Pointer.h"

namespace framework
{
    namespace generic
    {

        template <typename _Ty>
        class RawPointer
            : public Pointer<_Ty, RawPointer<_Ty> >
        {
        public:
            RawPointer()
                : addr_(NULL)
            {
            }

            RawPointer(
                _Ty * ptr)
                : addr_(ptr)
            {
            }

            template <typename _Ty1>
            RawPointer(
                RawPointer<_Ty1> const & ptr)
            {
                addr_ = ptr.addr_;
            }

            template <typename _Ty1>
            RawPointer & operator = (
                RawPointer<_Ty1> const & ptr)
            {
                addr_ = ptr.addr_;
                return *this;
            }

            friend bool operator == (
                RawPointer<_Ty> const & l, 
                RawPointer<_Ty> const & r)
            {
                return l.addr_ == r.addr_;
            }

            template <typename _Ty1>
            friend bool operator == (
                RawPointer<_Ty> const & l, 
                RawPointer<_Ty1> const & r)
            {
                return l.addr_ == r.addr_;
            }

            template <typename _Ty1>
            friend bool operator == (
                RawPointer<_Ty1> const & l, 
                RawPointer<_Ty> const & r);

            template <typename _Ty1>
            friend bool operator != (
                RawPointer<_Ty> const & l, 
                RawPointer<_Ty1> const & r)
            {
                return !(l.addr_ == r.addr_);
            }

            template <typename _Ty1>
            friend bool operator != (
                RawPointer<_Ty1> const & l, 
                RawPointer<_Ty> const & r);

            _Ty & operator*() const
            {
                return *addr_;
            }

        private:
            template <typename _Ty1>
            friend class RawPointer;

            _Ty * addr_;
        };

        template <typename _Ty>
        struct RawPointerTraits
        {
            typedef _Ty value_type;

            typedef RawPointer<_Ty> pointer;

            typedef RawPointer<_Ty const> const_pointer;

            typedef value_type & reference;

            typedef value_type const & const_reference;

            typedef size_t size_type;

            typedef ptrdiff_t difference_type;

            template <typename _Ty1>
            struct rebind
            {
                typedef RawPointerTraits<_Ty1> type;
            };
        };

        template <>
        struct RawPointerTraits<void>
        {
            typedef void value_type;

            typedef RawPointer<void> pointer;

            typedef RawPointer<void const> const_pointer;

            template <typename _Ty1>
            struct rebind
            {
                typedef RawPointerTraits<_Ty1> type;
            };
        };

    } // namespace generic
} // namespace framework

#endif // #ifndef _FRAMEWORK_GENERIC_RAW_POINTER_H_
