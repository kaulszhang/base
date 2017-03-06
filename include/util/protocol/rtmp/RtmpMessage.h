// RtmpMessage.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_H_

#include "util/protocol/Message.h"
#include "util/protocol/rtmp/RtmpMessageTraits.h"

#include <util/serialization/SplitMember.h>

namespace util
{
    namespace protocol
    {

        class RtmpMessageHeader
        {
        public:
            boost::uint16_t chunk;
            boost::uint8_t type;
            boost::uint32_t stream;
            boost::uint32_t timestamp;
            boost::uint32_t length;

            RtmpMessageHeader()
                : chunk(2)
                , type(0)
                , stream(0)
                , timestamp(0)
                , length(0)
            {
            }

            void id(
                boost::uint8_t i)
            {
                type = i;
            }

            boost::uint8_t id() const
            {
                return type;
            }

            boost::uint32_t data_size() const
            {
                return length;
            }

            void data_size(
                boost::uint32_t n)
            {
                length = n;
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void save(
                Archive & ar) const;

            template <typename Archive>
            void load(
                Archive & ar);
        };

        class RtmpMessageHeaderEx
            : public RtmpMessageHeader
        {
        public:
            boost::uint32_t chunk_size;
        };

        typedef Message<RtmpMessageTraits> RtmpMessage;

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_HEADER_H_
