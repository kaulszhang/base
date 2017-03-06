// RtmpMessageTraits.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_TRAITS_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_TRAITS_H_

#include "util/protocol/MessageTraits.h"

#include <util/archive/BigEndianBinaryIArchive.h>
#include <util/archive/BigEndianBinaryOArchive.h>

namespace util
{
    namespace protocol
    {

        class RtmpMessageHeader;
        class RtmpMessageHelper;
        class RtmpMessageContext;

        struct RtmpMessageTraits
            : MessageTraits
        {
            typedef boost::uint8_t id_type;

            typedef RtmpMessageHeader header_type;

            typedef util::archive::BigEndianBinaryIArchive<boost::uint8_t> i_archive_t;

            typedef util::archive::BigEndianBinaryOArchive<boost::uint8_t> o_archive_t;

            static size_t const max_size = 200;

            typedef RtmpMessageContext context_t;

            typedef RtmpMessageHelper helper_t;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_TRAITS_H_
