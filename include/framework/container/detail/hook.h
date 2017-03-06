// hook.h

#ifndef _FRAMEWORK_CONTAINER_DETAIL_HOOK_H_
#define _FRAMEWORK_CONTAINER_DETAIL_HOOK_H_

#include "framework/generic/NativePointer.h"

namespace framework
{
    namespace container
    {

        template <
            typename _Ty, 
            typename _Pt = framework::generic::NativePointerTraits<_Ty>
        >
        class Hook
        {
        public:
            typedef _Pt pointer_traits;

        public:
            _Ty * operator -> ()
            {
                return get();
            }

            _Ty const * operator -> () const
            {
                return get();
            }

            _Ty * get()
            {
                return static_cast<_Ty *>(this);
            }

            _Ty const * get() const
            {
                return static_cast<_Ty const *>(this);
            }

            void detach()
            {
            }
        };

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_DETAIL_HOOK_H_
