// RtspRequest.h

#ifndef _UTIL_PROTOCOL_RTSP_REQUEST_H_
#define _UTIL_PROTOCOL_RTSP_REQUEST_H_

#include "util/protocol/rtsp/RtspPacket.h"

namespace util
{
    namespace protocol
    {

        class RtspRequestHead
            : public RtspHead
        {
        public:
            enum MethodEnum
            {
                describe, 
                announce, 
                get_parameter, 
                set_parameter, 
                options, 
                redirect, 
                setup, 
                play, 
                record, 
                pause, 
                teardown, 
                invalid_method, 
            };

        public:
            RtspRequestHead(
                MethodEnum method = invalid_method)
                : method(method)
            {
                version = 0x00000100; // 1.0
            }

            RtspRequestHead(
                MethodEnum method, 
                std::string const & path, 
                size_t version = 0x00000100)
                : method(method)
                , path(path)
                , version(version) {}

        public:
            static std::string const method_str[];

        public:
            MethodEnum method;
            std::string path;
            size_t version;

        private:
            virtual bool get_line(
                std::string & line) const;

            virtual bool set_line(
                std::string const & line);
        };

        class RtspRequest
            : public RtspPacket
        {
        public:
            RtspRequest()
                : RtspPacket(head_)
            {
            }

            RtspRequestHead & head()
            {
                return head_;
            }

            RtspRequestHead const & head() const
            {
                return head_;
            }

        private:
            RtspRequestHead head_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_REQUEST_H_
