// MmsRequest.h

#ifndef _UTIL_PROTOCOL_MMS_REQUEST_H_
#define _UTIL_PROTOCOL_MMS_REQUEST_H_

#include "util/protocol/mms/MmsPacket.h"

namespace util
{
    namespace protocol
    {

        class MmsRequestHead
            : public MmsHead
        {
        public:
            enum MethodEnum
            {
                C2S_CANCELREADBLOCK             = 0x00030025,
                C2S_CLOSEFILE                   = 0x0003000d,
                C2S_CONNECT                     = 0x00030001,
                C2S_CONNECTFUNNEL               = 0x00030002,
                C2S_FUNNELINFO                  = 0x00030018,
                C2S_LOGGING                     = 0x00030032,
                C2S_OPENFILE                    = 0x00030005,
                C2S_PONG                        = 0x0003001b,
                C2S_READBLOCK                   = 0x00030015,
                C2S_SECURITYRESPONSE            = 0x0003001a,
                C2S_STARTPLAYING                = 0x00030007,
                C2S_STARTSTRIDING               = 0x00030028,
                C2S_STOPPLAYING                 = 0x00030009,
                C2S_STREAMSWITCH                = 0x00030033,
                C2S_INVALID_PACKET              = 0x00000000,
            };

        public:
            MmsRequestHead(
                MethodEnum method = C2S_INVALID_PACKET)
                : method(method)
            {
            }

        public:
            MethodEnum method;
        };

        class MmsRequest
            : public MmsPacket
        {
        public:
            MmsRequest()
                : MmsPacket(head_)
            {
            }

            MmsRequestHead & head()
            {
                return head_;
            }

            MmsRequestHead const & head() const
            {
                return head_;
            }

        private:
            MmsRequestHead head_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMS_REQUEST_H_
