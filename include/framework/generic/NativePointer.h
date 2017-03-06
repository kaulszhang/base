// RawPointer.h

#ifndef _FRAMEWORK_GENERIC_NATIVE_POINTER_H_
#define _FRAMEWORK_GENERIC_NATIVE_POINTER_H_

namespace framework
{
    namespace generic
    {

        template <typename _Ty>
        struct NativePointerTraits
        {
            typedef _Ty value_type;

            typedef value_type * pointer;

            typedef const value_type * const_pointer;

            typedef value_type & reference;

            typedef const value_type & const_reference;

            typedef size_t size_type;

            typedef ptrdiff_t difference_type;

            template <typename _Ty1>
            struct rebind
            {
                typedef NativePointerTraits<_Ty1> type;
            };
        };

        template <>
        struct NativePointerTraits<void>
        {
            typedef void value_type;

            typedef void * pointer;

            typedef const void * const_pointer;

            template <typename _Ty1>
            struct rebind
            {
                typedef NativePointerTraits<_Ty1> type;
            };
        };

    } // namespace generic
} // namespace framework

#endif // #ifndef _FRAMEWORK_GENERIC_NATIVE_POINTER_H_
