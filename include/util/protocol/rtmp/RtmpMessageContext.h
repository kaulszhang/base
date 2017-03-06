// RtmpMessageContext.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_CONTEXT_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_CONTEXT_H_

#include "util/protocol/rtmp/RtmpChunkHeader.h"
#include "util/protocol/rtmp/RtmpChunkMessage.h"

namespace util
{
    namespace protocol
    {

        class RtmpMessageHeader;
        struct RtmpMessageUserControl;

        struct RtmpStream
        {
            enum StatusEnum
            {
                stopped, 
                started, 
                eof, 
                dry, 
                buffering, 
            };

            RtmpStream()
                : status(stopped)
                , is_record(false)
            {
            }

            StatusEnum status;
            bool is_record;
        };

        class RtmpMessageOneContext
        {
        public:
            RtmpMessageOneContext();

        public:
            boost::uint32_t chunk_size() const
            {
                return chunk_size_;
            }

            void chunk_size(
                boost::uint32_t n);

            boost::uint32_t sequence() const
            {
                return seq_;
            }

            void acknowledgement(
                boost::uint32_t n);

            boost::uint32_t acknowledgement() const
            {
                return ack_;
            }

            void window_size(
                boost::uint32_t n);

            boost::uint32_t window_size() const
            {
                return win_;
            }

            void user_control(
                RtmpMessageUserControl const & msg);

        public:
            RtmpChunkMessage & chunk(
                boost::uint16_t cs_id);

        public:
            void stream_begin(
                boost::uint32_t i);

            void stream_end(
                boost::uint32_t i);

            void stream_status(
                boost::uint32_t i, 
                RtmpStream::StatusEnum s);

            void stream_is_record(
                boost::uint32_t i, 
                bool b);

            RtmpStream::StatusEnum stream_status(
                boost::uint32_t i) const;

            bool stream_is_record(
                boost::uint32_t i) const;

        protected:
            boost::uint32_t chunk_size_;
            boost::uint32_t win_;
            boost::uint32_t seq_;
            boost::uint32_t ack_;
            std::vector<RtmpChunkMessage> chunks_;
            std::vector<RtmpStream> streams_;
        };

        class RtmpMessageReadContext
            : public RtmpMessageOneContext
        {
        public:
            void from_chunk(
                RtmpMessageHeader & msg, 
                RtmpChunkHeader const & chunk);
        };

        class RtmpMessageWriteContext
            : public RtmpMessageOneContext
        {
        public:
            void to_chunk(
                RtmpMessageHeader const & msg, 
                RtmpChunkHeader & chunk);
        };

        class RtmpMessageContext
        {
        public:
            RtmpMessageReadContext read;
            RtmpMessageWriteContext write;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_CONTEXT_H_
