// Url.h

#ifndef _UTIL_SERIALIZATION_URL_H_
#define _UTIL_SERIALIZATION_URL_H_

#include <framework/string/Url.h>

namespace util
{
    namespace serialization
    {

        template <
            typename Archive
        >
        void serialize(
            Archive & ar, 
            framework::string::Url& t)
        {
            boost::system::error_code ec;
            std::string value;
            ar & value;
            ec = t.from_string(value);
            if (ec)
                ar.fail();
        }
    }
}

#endif // _UTIL_SERIALIZATION_NETNAME_H_
