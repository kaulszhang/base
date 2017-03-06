// ordered.h

#ifndef _FRAMEWORK_CONTAINER_DETAIL_ORDERED_H_
#define _FRAMEWORK_CONTAINER_DETAIL_ORDERED_H_

namespace framework
{
    namespace container
    {

        struct ordered_non_unique_tag {};
        struct ordered_unique_tag {};
        struct lower_unbounded_tag {};
        struct upper_unbounded_tag {};
        struct both_unbounded_tag {};
        struct none_unbounded_tag {};
        struct unbounded_type {};

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_DETAIL_ORDERED_H_
