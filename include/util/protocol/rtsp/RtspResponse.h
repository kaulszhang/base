// RtspResponse.h

#ifndef _UTIL_PROTOCOL_RTSP_RESPONSE_H_
#define _UTIL_PROTOCOL_RTSP_RESPONSE_H_

#include "util/protocol/rtsp/RtspPacket.h"

namespace util
{
    namespace protocol
    {
        class RtspResponseHead
            : public RtspHead
        {
        public:
            size_t version;
            size_t err_code;
            std::string err_msg;

        public:
            RtspResponseHead(
                size_t err_code = 200, 
                size_t version = 0x00000100)
                : version(version)
                , err_code(err_code)
            {
            }

            RtspResponseHead(
                size_t err_code, 
                std::string const & err_msg, 
                size_t version = 0x00000100)
                : version(version)
                , err_code(err_code)
                , err_msg(err_msg)
            {
            }

        private:
            virtual bool get_line(
                std::string & line) const;

            virtual bool set_line(
                std::string const & line);
        };

        class RtspResponse
            : public RtspPacket
        {
        public:
            RtspResponse()
                : RtspPacket(head_)
            {
            }

            RtspResponseHead & head()
            {
                return head_;
            }

            RtspResponseHead const & head() const
            {
                return head_;
            }

        private:
            RtspResponseHead head_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_RESPONSE_H_
