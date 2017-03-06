// RtmpMessage.hpp

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_HPP_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_HPP_

#include "util/protocol/rtmp/RtmpMessage.h"
#include "util/protocol/rtmp/RtmpMessageContext.h"
#include "util/protocol/rtmp/RtmpMessageHelper.h"
#include "util/protocol/Message.hpp"

namespace util
{
    namespace protocol
    {

        template <typename Archive>
        void RtmpMessageHeader::save(
            Archive & ar) const
        {
            RtmpMessageContext * ctx = (RtmpMessageContext *)ar.context();
            RtmpChunkHeader chunk;
            ctx->write.to_chunk(*this, chunk);
            ar & chunk;
        }

        template <typename Archive>
        void RtmpMessageHeader::load(
            Archive & ar)
        {
            RtmpMessageContext * ctx = (RtmpMessageContext *)ar.context();
            RtmpChunkHeader chunk;
            ar & chunk;
            ctx->read.from_chunk(*this, chunk);
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_HPP_
