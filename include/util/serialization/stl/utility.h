// utility.h

#ifndef _UTIL_SERIALIZATION_STL_UTILITY_H_
#define _UTIL_SERIALIZATION_STL_UTILITY_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"

#include <boost/type_traits/remove_const.hpp>

#include <utility>

namespace std
{

    // pair
    template<class Archive, class _Ty1, class _Ty2>
    inline void serialize(
        Archive & ar,
        std::pair<_Ty1, _Ty2> & p)
    {
        typedef typename boost::remove_const<_Ty1>::type typef;
        ar & util::serialization::make_nvp("first", const_cast<typef &>(p.first));
        ar & util::serialization::make_nvp("second", p.second);
    }

} // namespace std

#endif // _UTIL_SERIALIZATION_STL_UTILITY_H_
