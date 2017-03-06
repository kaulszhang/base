// RtmpChunkParser.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpChunkParser.h"
#include "util/protocol/rtmp/RtmpMessageContext.h"
#include "util/protocol/rtmp/RtmpMessage.h"

namespace util
{
    namespace protocol
    {

        RtmpChunkParser::RtmpChunkParser(
            RtmpMessageContext * ctx)
            : ctx_(ctx)
        {
        }

        void RtmpChunkParser::parse(
            boost::asio::const_buffer const & buf)
        {
            boost::uint8_t const * b = 
                boost::asio::buffer_cast<boost::uint8_t const *>(buf);
            boost::uint8_t const * e = 
                b + boost::asio::buffer_size(buf);
            switch (step_) {
                case 0:
                    size_ = 1;
                    step_ = 1;
                    break;
                case 1:
                    chunk_.one_byte = *b;
                    size_ = chunk_.size() + chunk_.msgh_size();
                    step_ = 2;
                    break;
                case 2:
                    {
                        boost::uint8_t const * p = b;
                        if (chunk_.size() == 1) {
                        } else if (chunk_.size() == 2) {
                            chunk_.cs_id1 = p[1];
                        } else {
                            chunk_.cs_id2 = (boost::uint16_t)p[1] << 8 | p[2];
                        }
                        p += chunk_.size();
                        if (chunk_.fmt < 3) {
                            boost::uint32_t t = ((boost::uint32_t)p[0] << 16 | (boost::uint32_t)p[1] << 8 | p[2]);
                            if (t == 0xffffff) {
                                e += 4;
                            }
                            p += 3;
                        }
                        boost::uint16_t c = chunk_.cs_id();
                        RtmpChunkMessage & rchunk(ctx_->read.chunk(c));
                        boost::uint8_t t = 0;
                        if (chunk_.fmt < 2) {
                            boost::uint32_t l = ((boost::uint32_t)p[0] << 16 | (boost::uint32_t)p[1] << 8 | p[2]);
                            t = p[3];
                            e += rchunk.left_size(l, t, ctx_->read.chunk_size());
                        } else {
                            t = rchunk.message_type_id;
                            e += rchunk.left_size(ctx_->read.chunk_size());
                        }
                        ok_ = true;
                        size_ = e - b;
                        msg_def_ = RtmpMessage::find_msg(t);
                        assert(msg_def_);
                        step_ = 3;
                        break;
                    }
                default:
                    assert(false);
            }
        }

    } // namespace protocol
} // namespace util
