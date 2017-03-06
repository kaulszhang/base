// RtmpChunkHeader.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_HEADER_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_HEADER_H_

#include <util/serialization/NumberBits24.h>

#include <framework/system/NumberBits24.h>
#include <framework/system/BytesOrder.h>

namespace util
{
    namespace protocol
    {

        struct RtmpChunkBasicHeader
        {
        public:
            union
            {
                struct {
#ifdef   BOOST_BIG_ENDIAN
                    boost::uint8_t fmt : 2;
                    boost::uint8_t cs_id0 : 6;
#else
                    boost::uint8_t cs_id0 : 6;
                    boost::uint8_t fmt : 2;
#endif
                };
                boost::uint8_t one_byte;
            };
            boost::uint8_t cs_id1;
            boost::uint16_t cs_id2;

        public:
            RtmpChunkBasicHeader(
                boost::uint8_t fmt = 0, 
                boost::uint16_t id = 2)
                : one_byte((fmt << 6) | ((boost::uint8_t)id & 0x3f))
                , cs_id1(0)
                , cs_id2(0)
            {
                cs_id(id);
            }
            
            boost::uint8_t size() const
            {
                return cs_id0 == 0 ? 2 : (cs_id0 == 1 ? 3 : 1);
            }

            boost::uint8_t msgh_size() const
            {
                boost::uint8_t n[4] = {11, 7, 3, 0};
                return n[fmt];
            }

            boost::uint16_t cs_id() const
            {
                return cs_id0 < 2 ? ((cs_id0 == 0 ? cs_id1 : cs_id2) + 64) : cs_id0;
            }

            void cs_id(
                boost::uint16_t id)
            {
                cs_id0 = id;
                if (cs_id0 != id) {
                    id -= 64;
                    cs_id1 = (boost::uint8_t)id;
                    if (cs_id1 != id) {
                        cs_id0 = 1;
                        cs_id1 = 0;
                        cs_id2 = id;
                    } else {
                        cs_id0 = 0;
                    }
                }
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & one_byte;
                if (cs_id0 == 0) {
                    ar & cs_id1;
                } else if (cs_id0 == 1) {
                    ar & cs_id2;
                }
            }
        };

        struct RtmpChunkMessageHeader
        {
            framework::system::UInt24 timestamp; // timestamp or timestamp delta
            framework::system::UInt24 message_length;
            boost::uint8_t message_type_id;
            boost::uint32_t message_stream_id;

            RtmpChunkMessageHeader()
                : message_type_id(0)
                , message_stream_id(boost::uint32_t(-1))
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                boost::uint16_t fmt = 
                    (boost::uint16_t)(intptr_t)ar.context();
                switch (fmt)
                {
                case 0:
                    ar & timestamp;
                    ar & message_length;
                    ar & message_type_id;
                    {
                        boost::uint32_t message_stream_id = 
                            framework::system::BytesOrder::rotate(this->message_stream_id);
                        ar & message_stream_id;
                        this->message_stream_id = 
                            framework::system::BytesOrder::rotate(message_stream_id);
                    }
                    break;
                case 1:
                    ar & timestamp;
                    ar & message_length;
                    ar & message_type_id;
                    break;
                case 2:
                    ar & timestamp;
                    break;
                case 3:
                    break;
                }
            }
        };

        struct RtmpChunkHeader
            : RtmpChunkBasicHeader
            , RtmpChunkMessageHeader
        {
            boost::uint32_t extended_timestamp;
            boost::uint32_t calc_timestamp;

            RtmpChunkHeader()
                : extended_timestamp(0)
                , calc_timestamp(0)
            {
            }

            RtmpChunkHeader const & add(
                RtmpChunkHeader const & r);

            RtmpChunkHeader const & dec(
                RtmpChunkHeader const & r);

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                RtmpChunkBasicHeader::serialize(ar);
                void * ctx = ar.context();
                ar.context((void *)fmt);
                RtmpChunkMessageHeader::serialize(ar);
                ar.context(ctx);

                if (fmt != 3 && timestamp == 0x00ffffff) {
                    ar & extended_timestamp;
                } else {
                    extended_timestamp = timestamp;
                }
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_HEADER_H_
