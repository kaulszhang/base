// deque.h

#ifndef _UTIL_SERIALIZATION_STL_DEQUE_H_
#define _UTIL_SERIALIZATION_STL_DEQUE_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"

#include <deque>

namespace std
{

    template<
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void serialize(
        Archive & ar,
        std::deque<_Ty, _Alloc> & t)
    {
        util::serialization::serialize_collection(ar, t);
    }

} // namespace std

#endif // _UTIL_SERIALIZATION_STL_DEQUE_H_
