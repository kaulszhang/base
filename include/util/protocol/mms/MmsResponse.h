// MmsResponse.h

#ifndef _UTIL_PROTOCOL_MMS_RESPONSE_H_
#define _UTIL_PROTOCOL_MMS_RESPONSE_H_

#include "util/protocol/mms/MmsPacket.h"

namespace util
{
    namespace protocol
    {
        class MmsResponseHead
            : public MmsHead
        {
        public:
            enum MethodEnum
            {
                S2C_PING                        = 0x0004001b,
                S2C_CONNECTEDEX                 = 0x00040001,
                S2C_CONNECTEDFUNNEL             = 0x00040002,
                S2C_DISCONNECTEDCONFUNNEL       = 0x00040003,
                S2C_ENDOFSTREAM                 = 0x0004001e,
                S2C_FUNNELINFO                  = 0x00040015,
                S2C_OPENFILE                    = 0x00040006,
                S2C_READBLOCK                   = 0x00040011,
                S2C_REDIRECT                    = 0x00040022,
                S2C_STARTEDPLAYING              = 0x00040005,
                S2C_STARTSTRIDING               = 0x0004000a,
                S2C_STREAMCHANGE                = 0x00040020,
                S2C_STREAMSWITCH                = 0x00040021,
                S2C_SECURITYCHALLENGE           = 0x0004001a,
                S2C_INVALID_PACKET              = 0x00000000,
            };

            boost::uint32_t err_code;
            MethodEnum method;

        public:
            MmsResponseHead(
                MethodEnum method = S2C_INVALID_PACKET)
                : method( method )
            {
            }

        };

        class MmsResponse
            : public MmsPacket
        {
        public:
            MmsResponse()
                : MmsPacket(head_)
            {
            }

            MmsResponseHead & head()
            {
                return head_;
            }

            MmsResponseHead const & head() const
            {
                return head_;
            }

        private:
            MmsResponseHead head_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMS_RESPONSE_H_
