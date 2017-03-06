// RtmpChunkMessage.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_MESSAGE_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_MESSAGE_H_

#include "util/protocol/rtmp/RtmpChunkHeader.h"
#include "util/protocol/MessageDefine.h"

#include <util/buffers/BuffersCopy.h>

namespace util
{
    namespace protocol
    {

        struct RtmpChunkMessage
            : RtmpChunkHeader
        {
            StreamBuffer data;
            boost::uint32_t left;

            RtmpChunkMessage()
                : left(0)
            {
            }

            bool put_data(
                StreamBuffer & buf, 
                boost::uint32_t chunk_size)
            {
                boost::uint32_t size = buf.size();
                util::buffers::buffers_copy(data.prepare(size), buf.data());
                buf.consume(size);
                data.commit(size);
                return put_data(chunk_size);
            }

            bool put_data(
                boost::uint32_t chunk_size)
            {
                if (left <= chunk_size) {
                    left = 0;
                } else {
                    left -= chunk_size;
                }
                return left == 0;
            }

            // return size of next chunk, not contain the chunk head
            boost::uint32_t left_size(
                boost::uint32_t new_size, 
                boost::uint8_t new_type, 
                boost::uint32_t chunk_size)
            {
                assert(left == 0);
                left = new_size;
                message_type_id = new_type;
                boost::uint32_t size = left;
                if (size > chunk_size) {
                    size = chunk_size;
                }
                return size;
            }

            boost::uint32_t left_size(
                boost::uint32_t chunk_size)
            {
                boost::uint32_t size = left;
                if (size == 0) {
                    left = message_length;
                    size = message_length;
                }
                if (size > chunk_size) {
                    size = chunk_size;
                }
                return size;
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_HEADER_H_
