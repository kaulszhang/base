// RtmpMessageDataProtocolControl.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_PROTOCOL_CONTROL_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_PROTOCOL_CONTROL_H_

#include "util/protocol/rtmp/RtmpMessageData.h"

namespace util
{
    namespace protocol
    {

        struct RtmpMessageDataSetChunkSize
            : RtmpMessageData<RtmpMessageDataSetChunkSize, RCMT_SetChunkSize>
        {
        public:
            boost::uint32_t chunk_size;

        public:
            RtmpMessageDataSetChunkSize(
                boost::uint32_t chunk_size = 128)
                : chunk_size(chunk_size)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & chunk_size;
            }
        };

        struct RtmpMessageDataAbortMessage
            : RtmpMessageData<RtmpMessageDataAbortMessage, RCMT_AbortMessage>
        {
        public:
            boost::uint32_t chunk_stream_id;

        public:
            RtmpMessageDataAbortMessage(
                boost::uint32_t chunk_stream_id = 0)
                : chunk_stream_id(chunk_stream_id)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & chunk_stream_id;
            }
        };

        struct RtmpMessageDataAcknowledgement
            : RtmpMessageData<RtmpMessageDataAcknowledgement, RCMT_Acknowledgement>
        {
        public:
            boost::uint32_t sequence_number;

        public:
            RtmpMessageDataAcknowledgement(
                boost::uint32_t sequence_number = 0)
                : sequence_number(sequence_number)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & sequence_number;
            }
        };

        struct RtmpMessageDataWindowAcknowledgementSize
            : RtmpMessageData<RtmpMessageDataWindowAcknowledgementSize, RCMT_WindowAcknowledgementSize>
        {
        public:
            boost::uint32_t acknowledgement_window_size;

        public:
            RtmpMessageDataWindowAcknowledgementSize(
                boost::uint32_t acknowledgement_window_size = 0)
                : acknowledgement_window_size(acknowledgement_window_size)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & acknowledgement_window_size;
            }
        };

        struct RtmpMessageDataSetPeerBandwidth
            : RtmpMessageData<RtmpMessageDataSetPeerBandwidth, RCMT_SetPeerBandwidth>
        {
        public:
            boost::uint32_t acknowledgement_window_size;
            boost::uint8_t limit_type;

        public:
            RtmpMessageDataSetPeerBandwidth(
                boost::uint32_t acknowledgement_window_size = 0, 
                boost::uint8_t limit_type = 0)
                : acknowledgement_window_size(acknowledgement_window_size)
                , limit_type(limit_type)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & acknowledgement_window_size;
                ar & limit_type;
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_PROTOCOL_CONTROL_H_
