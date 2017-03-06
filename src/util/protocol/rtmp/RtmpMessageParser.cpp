// RtmpMessageParser.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpMessageParser.h"
#include "util/protocol/rtmp/RtmpMessageContext.h"
#include "util/protocol/rtmp/RtmpMessage.h"

namespace util
{
    namespace protocol
    {

        RtmpMessageParser::RtmpMessageParser(
            RtmpMessageContext * ctx)
            : ctx_(ctx)
        {
        }

        void RtmpMessageParser::parse(
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
                        boost::uint32_t l = 0;
                        boost::uint8_t t = 0;
                        if (chunk_.fmt < 2) {
                            l = ((boost::uint32_t)p[0] << 16 | (boost::uint32_t)p[1] << 8 | p[2]);
                            t = p[3];
                        } else {
                            boost::uint16_t c = chunk_.cs_id();
                            RtmpChunkHeader & rchunk(ctx_->read.chunk(c));
                            l = rchunk.message_length;
                            t = rchunk.message_type_id;
                        }
                        e += l;
                        while (l > ctx_->read.chunk_size()) {
                            e += chunk_.size();
                            l -= ctx_->read.chunk_size();
                        }
                        ok_ = true;
                        size_ = e - b;
                        msg_def_ = RtmpMessage::find_msg(t);
                        step_ = 3;
                        break;
                    }
                default:
                    assert(false);
            }
        }

    } // namespace protocol
} // namespace util
