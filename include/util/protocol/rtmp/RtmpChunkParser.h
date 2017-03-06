// RtmpChunkParser.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_PARSER_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_PARSER_H_

#include "util/protocol/MessageParser.h"
#include "util/protocol/rtmp/RtmpChunkHeader.h"

namespace util
{
    namespace protocol
    {

        class RtmpMessageContext;

        class RtmpChunkParser
            : public MessageParser
        {
        public:
            RtmpChunkParser(
                RtmpMessageContext * ctx);

        public:
            virtual void parse(
                boost::asio::const_buffer const & buf);

        private:
            RtmpMessageContext * ctx_;
            RtmpChunkHeader chunk_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_CHUNK_PARSER_H_
