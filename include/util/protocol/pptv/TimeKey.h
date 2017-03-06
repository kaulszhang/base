// TimeKey.h

#ifndef _UTIL_PROTOCOL_PPTV_TIME_KEY_H_
#define _UTIL_PROTOCOL_PPTV_TIME_KEY_H_

namespace util
{
    namespace protocol
    {

        namespace pptv
        {

            std::string gen_key_from_time(
                unsigned int time_now);

        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_PPTV_TIME_KEY_H_
