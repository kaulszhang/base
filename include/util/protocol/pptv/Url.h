// Url.h

#ifndef _UTIL_PROTOCOL_PPTV_URL_H_
#define _UTIL_PROTOCOL_PPTV_URL_H_

namespace util
{
    namespace protocol
    {

        namespace pptv
        {

            std::string url_encode(
                std::string const & str, 
                std::string const & key);

            std::string url_decode(
                std::string const & str, 
                std::string const & key);

        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_PPTV_URL_H_
