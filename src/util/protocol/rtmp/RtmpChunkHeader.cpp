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
            // ÿ������ȥMessage������ת��Ϊ Chunk �ĵ��ã���һ�� r.message_length == 0����ʱ��ֻ������ȷ��fmt������ֵ����
            if (r.message_length == 0) {
                // ��Ϊ r.message_length δ֪�����Բ��� fmt Ϊ 2,3
                if (r.message_stream_id == message_stream_id) {
                    fmt = 1;
                } else {
                    fmt = 0;
                }
                // message_length ǿ������Ϊ 0���ڶ��ξͲ������� fmt Ϊ 2,3 �� Chunk ��
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
