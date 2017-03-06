// Url.cpp

#include "util/Util.h"
#include "util/protocol/pptv/Url.h"
#include "util/protocol/pptv/Base64.h"

#include <framework/string/Url.h>

namespace util
{
    namespace protocol
    {

        namespace pptv
        {

            std::string url_encode(
                std::string const & str, 
                std::string const & key)
            {
                return framework::string::Url::encode(base64_encode(str, key), "/.");
            }

            std::string url_decode(
                std::string const & str, 
                std::string const & key)
            {
                return base64_decode(framework::string::Url::decode(str), key);
            }

        }

    } // namespace protocol
} // namespace util

