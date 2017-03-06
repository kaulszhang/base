// RtmpMessageData.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_H_

#include "util/protocol/rtmp/RtmpMessageTraits.h"
#include "util/protocol/MessageData.h"

namespace util
{
    namespace protocol
    {

        enum RtmpMessageType
        {
            RCMT_SetChunkSize = 1, 
            RCMT_AbortMessage = 2, 
            RCMT_Acknowledgement = 3, 
            RCMT_UserControl = 4, 
            RCMT_WindowAcknowledgementSize = 5, 
            RCMT_SetPeerBandwidth = 6, 

            RCMT_AudioMessage = 8, 
            RCMT_VideoMessage = 9, 

            RCMT_DataMessage3 = 15, 
            RCMT_SharedObjectMessage3 = 16, 
            RCMT_CommandMessage3 = 17, 
            RCMT_DataMessage0 = 18, 
            RCMT_SharedObjectMessage0 = 19, 
            RCMT_CommandMessage0 = 20, 

            RCMT_AggregateMessage = 22, 
        };

        template <
            typename T, 
            boost::uint8_t id
        >
        struct RtmpMessageData
            : MessageData<RtmpMessageTraits, T, id>
        {
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_DATA_H_
