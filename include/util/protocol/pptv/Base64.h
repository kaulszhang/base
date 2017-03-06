// Base64.h

#ifndef _UTIL_PROTOCOL_PPTV_BASE_64_H_
#define _UTIL_PROTOCOL_PPTV_BASE_64_H_

namespace util
{
    namespace protocol
    {

        namespace pptv
        {

            std::string base64_encode(
                std::string const & str, 
                std::string const & key);

            std::string base64_decode(
                std::string const & str, 
                std::string const & key);

        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_PPTV_BASE_64_H_
