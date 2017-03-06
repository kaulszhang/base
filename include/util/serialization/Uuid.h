// Uuid.h

#ifndef _UTIL_SERIALIZATION_STRING_GUID_H_
#define _UTIL_SERIALIZATION_STRING_GUID_H_

#include "util/serialization/NVPair.h"
#include "util/serialization/SplitFree.h"
#include "util/serialization/Array.h"

#include <framework/string/Uuid.h>

namespace framework 
{
    namespace string 
    {

        template <typename Archive>
        void serialize(Archive & ar, framework::string::Uuid & t)
        {
            util::serialization::split_free(ar, t);
        }

        template <typename Archive>
        void load(Archive & ar, framework::string::Uuid & t)
        {
            framework::string::UUID uuid;
            ar & SERIALIZATION_NVP_1(uuid, Data1)
                & SERIALIZATION_NVP_1(uuid, Data2)
                & SERIALIZATION_NVP_1(uuid, Data3)
                & util::serialization::make_nvp("Data4", 
                    framework::container::make_array(uuid.Data4, sizeof(uuid.Data4)));
            if (ar)
                t.assign(uuid);
        };

        template <typename Archive>
        void save(Archive & ar, framework::string::Uuid const & t)
        {
            framework::string::UUID const & uuid = t.data();
            ar & SERIALIZATION_NVP_1(uuid, Data1)
                & SERIALIZATION_NVP_1(uuid, Data2)
                & SERIALIZATION_NVP_1(uuid, Data3)
                & util::serialization::make_nvp("Data4", 
                    framework::container::make_array(uuid.Data4, sizeof(uuid.Data4)));
        }
    }
}

#endif // _UTIL_SERIALIZATION_STRING_GUID_H_
