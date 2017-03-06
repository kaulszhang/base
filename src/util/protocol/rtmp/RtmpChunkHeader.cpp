// RtmpChunkHeader.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpChunkHeader.h"

namespace util
{
    namespace protocol
    {

        RtmpChunkHeader const & RtmpChunkHeader::add(
            RtmpChunkHeader const & r)
        {
            switch (r.fmt) {
                case 0:
                    timestamp = r.timestamp;
                    message_length = r.message_length;
                    message_type_id = r.message_type_id;
                    message_stream_id = r.message_stream_id;
                    extended_timestamp = r.extended_timestamp;
                    calc_timestamp = r.extended_timestamp;
                    break;
                case 1:
                    timestamp = r.timestamp;
                    message_length = r.message_length;
                    message_type_id = r.message_type_id;
                    extended_timestamp = r.extended_timestamp;
                    calc_timestamp += r.extended_timestamp;
                    break;
                case 2:
                    timestamp = r.timestamp;
                    extended_timestamp = r.extended_timestamp;
                    calc_timestamp += r.extended_timestamp;
                    break;
                case 3:
                    calc_timestamp += extended_timestamp;
                    break;
            }
            return *this;
        }

        RtmpChunkHeader const & RtmpChunkHeader::dec(
            RtmpChunkHeader const & r)
        {
            // 每个发出去Message有两次转换为 Chunk 的调用，第一次 r.message_length == 0，这时候只生成正确的fmt，其他值不变
            if (r.message_length == 0) {
                // 因为 r.message_length 未知，所以不能 fmt 为 2,3
                if (r.message_stream_id == message_stream_id) {
                    fmt = 1;
                } else {
                    fmt = 0;
                }
                // message_length 强制设置为 0，第二次就不会生成 fmt 为 2,3 的 Chunk 了
                message_length = 0;
                return *this;
            }
            if (r.message_stream_id == message_stream_id) {
                if (r.message_type_id == message_type_id 
                    && r.message_length == message_length) {
                        if (r.calc_timestamp == calc_timestamp + extended_timestamp) {
                            fmt = 3;
                        } else {
                            fmt = 2;
                            extended_timestamp = r.calc_timestamp - calc_timestamp;
                        }
                } else {
                    fmt = 1;
                    message_length = r.message_length;
                    message_type_id = r.message_type_id;
                    extended_timestamp = r.calc_timestamp - calc_timestamp;
                }
            } else {
                fmt = 0;
                message_length = r.message_length;
                message_type_id = r.message_type_id;
                message_stream_id = r.message_stream_id;
                extended_timestamp = r.calc_timestamp;
            }
            calc_timestamp = r.calc_timestamp;
            if (extended_timestamp & 0xff000000) {
                //timestamp = 0x00ffffff;
                fmt = 0;
                timestamp = calc_timestamp;
                extended_timestamp = calc_timestamp;
            } else {
                timestamp = extended_timestamp;
            }
            return *this;
        }

    } // namespace protocol
} // namespace util
