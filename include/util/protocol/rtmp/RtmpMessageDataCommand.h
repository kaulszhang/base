// RtmpCommandMessage.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_COMMAND_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_COMMAND_H_

#include "util/protocol/rtmp/RtmpMessageData.h"
#include "util/protocol/rtmp/RmtpAmfType.h"

namespace util
{
    namespace protocol
    {

        /* Section 6.2
           User Control messages SHOULD use message stream ID 0 (known as the
           control stream) and, when sent over RTMP Chunk Stream, be sent on
           chunk stream ID 2. User Control messages are effective at the point
           they are received in the stream; their timestamps are ignored.
         */

        struct RtmpCommandMessage
        {
            RtmpAmfValue CommandName;
            RtmpAmfValue TransactionID;
            RtmpAmfValue CommandObject;
            std::vector<RtmpAmfValue> OptionalArguments;

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void load(
                Archive & ar)
            {
                ar & CommandName;
                ar & TransactionID;
                ar & CommandObject;

                RtmpAmfValue Argument;
                OptionalArguments.clear();

                if (!ar) {
                    return;
                }

                while (ar & Argument) {
                    OptionalArguments.push_back(Argument);
                }
                ar.clear();
            }

            template <typename Archive>
            void save(
                Archive & ar) const
            {
                ar & CommandName;
                ar & TransactionID;
                ar & CommandObject;

                for (size_t i = 0; i < OptionalArguments.size(); ++i) {
                    ar & OptionalArguments[i];
                }
            }
        };

        struct RtmpCommandMessage0
            : RtmpMessageData<RtmpCommandMessage0, RCMT_CommandMessage0>
            , RtmpCommandMessage
        {
        };

        struct RtmpCommandMessage3
            : RtmpMessageData<RtmpCommandMessage3, RCMT_CommandMessage3>
            , RtmpCommandMessage
        {
            boost::uint8_t _undefined;

            RtmpCommandMessage3()
                : _undefined(0)
            {
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & _undefined;
                RtmpCommandMessage::serialize(ar);
            }
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_COMMAND_H_
