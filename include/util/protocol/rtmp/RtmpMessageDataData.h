// RtmpMessageDataData.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_DATA_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_DATA_H_

#include "util/protocol/rtmp/RtmpMessageData.h"
#include "util/protocol/rtmp/RmtpAmfType.h"

namespace util
{
    namespace protocol
    {

        struct RtmpDataMessage
        {
            RtmpAmfValue Name;
            RtmpAmfValue Value;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & Name;
                ar & Value;
            }
        };

        struct RtmpDataMessage0
            : RtmpMessageData<RtmpDataMessage0, RCMT_DataMessage0>
            , RtmpDataMessage
        {
            static MessageDefine::ClassEnum const static_cls = MessageDefine::data_message;
        };

        struct RtmpDataMessage3
            : RtmpMessageData<RtmpDataMessage3, RCMT_DataMessage3>
            , RtmpDataMessage
        {
            static MessageDefine::ClassEnum const static_cls = MessageDefine::data_message;

            boost::uint8_t _undefined;

            RtmpDataMessage3()
                : _undefined(0)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & _undefined;
                RtmpDataMessage::serialize(ar);
            }
        };

        struct RtmpVideoMessage
            : RtmpMessageData<RtmpVideoMessage, RCMT_VideoMessage>
        {
            static MessageDefine::ClassEnum const static_cls = MessageDefine::data_message;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
            }
        };

        struct RtmpAudioMessage
            : RtmpMessageData<RtmpAudioMessage, RCMT_AudioMessage>
        {
            static MessageDefine::ClassEnum const static_cls = MessageDefine::data_message;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
            }
        };

        struct RtmpAggregateMessage
            : RtmpMessageData<RtmpAggregateMessage, RCMT_AggregateMessage>
        {
            static MessageDefine::ClassEnum const static_cls = MessageDefine::data_message;

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_COMMAND_H_
