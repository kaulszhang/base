// RtmpMessageHelper.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_HELPER_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_HELPER_H_

#include "util/protocol/rtmp/RtmpStreamBuffer.h"
#include "util/protocol/rtmp/RtmpMessageContext.h"
#include "util/protocol/MessageHelper.h"

namespace util
{
    namespace protocol
    {

        class RtmpMessageHelper
            : public MessageHelper<RtmpMessageTraits>
        {
        public:
            RtmpMessageHelper(
                i_archive_t & ar, 
                header_type & header, 
                context_t * ctx)
                : MessageHelper<RtmpMessageTraits>(ar, header, ctx)
                , buf_(*ar.rdbuf(), header, ctx->read.chunk_size())
            {
            }

            RtmpMessageHelper(
                o_archive_t & ar, 
                header_type const & header, 
                context_t * ctx)
                : MessageHelper<RtmpMessageTraits>(ar, header, ctx)
                , buf_(*ar.rdbuf(), header, ctx->write.chunk_size())
            {
            }

            ~RtmpMessageHelper()
            {
                filling_ = buf_.filling();
                if (ia_) {
                    ia_->rdbuf(&buf_.next_layer());
                } else {
                    oa_->rdbuf(&buf_.next_layer());
                }
            }

        public:
            void begin_data()
            {
                MessageHelper<RtmpMessageTraits>::begin_data();
                if (ia_) {
                    ia_->rdbuf(&buf_);
                } else {
                    oa_->rdbuf(&buf_);
                }
            }

        private:
            RtmpStreamBuffer<boost::uint8_t> buf_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_MESSAGE_HELPER_H_
