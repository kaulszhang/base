// NetName.h

#ifndef _UTIL_SERIALIZATION_NETNAME_H_
#define _UTIL_SERIALIZATION_NETNAME_H_

#include <framework/network/NetName.h>

namespace util
{
    namespace serialization
    {

        template <
            typename Archive
        >
        void serialize(
            Archive & ar, 
            framework::network::NetName & t)
        {
            boost::system::error_code ec;
            std::string value;
            ar & value;
            t.from_string(value);
            if (ec)
                ar.fail();
        }
    }
}

#endif // _UTIL_SERIALIZATION_NETNAME_H_
