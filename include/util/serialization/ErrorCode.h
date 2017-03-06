// ErrorCode.h

#ifndef _UTIL_SERIALIZATION_ERROR_CODE_H_
#define _UTIL_SERIALIZATION_ERROR_CODE_H_

#include "util/serialization/NVPair.h"
#include "util/serialization/SplitFree.h"

#include <boost/system/error_code.hpp>

namespace boost
{
    namespace system
    {

        template <class Archive>
        void save(
            Archive & ar, 
            boost::system::error_code const & t)
        {
            std::string category = t.category().name();
            int value = t.value();
            ar << SERIALIZATION_NVP(category) << SERIALIZATION_NVP(value);
        }

        template <class Archive>
        void load(
            Archive & ar, 
            boost::system::error_code & t)
        {
            std::string category;
            int value;
            ar >> SERIALIZATION_NVP(category) >> SERIALIZATION_NVP(value);
            t.assign(value, boost::system::error_category::find_category(category.c_str()));
        }

        template <class Archive>
        void serialize(
            Archive & ar, 
            boost::system::error_code & t)
        {
            util::serialization::split_free(ar, t);
        }

    }  // namespace system
} // namespace boost

#endif // _UTIL_SERIALIZATION_ERROR_CODE_H_
