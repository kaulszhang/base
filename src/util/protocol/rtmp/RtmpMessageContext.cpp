// RtmpMessageOneContext.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpMessageContext.h"
#include "util/protocol/rtmp/RtmpMessage.h"
#include "util/protocol/rtmp/RtmpMessageDataUserControl.h"

namespace util
{
    namespace protocol
    {

        RtmpMessageOneContext::RtmpMessageOneContext()
            : chunk_size_(128)
            , win_(250000)
            , seq_(0)
            , ack_(0)
        {
            streams_.resize(1);
            streams_[0].status = RtmpStream::started;
        }

        void RtmpMessageOneContext::chunk_size(
            boost::uint32_t n)
        {
            chunk_size_ = n;
        }

        void RtmpMessageOneContext::acknowledgement(
            boost::uint32_t n)
        {
            ack_ = n;
        }

        void RtmpMessageOneContext::window_size(
            boost::uint32_t n)
        {
            win_ = n;
        }

        void RtmpMessageOneContext::user_control(
            RtmpMessageUserControl const & msg)
        {
            boost::uint32_t stream = msg._union[0];
            switch (msg.event_type) {
                case RUCE_StreamBegin:
                    stream_status(stream, RtmpStream::started);
                    break;
                case RUCE_StreamEOF:
                    stream_status(stream, RtmpStream::eof);
                    break;
                case RUCE_StreamDry:
                    stream_status(stream, RtmpStream::dry);
                    break;
                case RUCE_BufferEmpty:
                    stream_status(stream, RtmpStream::buffering);
                    break;
                case RUCE_BufferReady:
                    stream_status(stream, RtmpStream::started);
                    break;
                case RUCE_StreamIsRecorded:
                    stream_is_record(stream, true);
                    break;
                default:
                    break;
            }
        }

        RtmpChunkMessage & RtmpMessageOneContext::chunk(
            boost::uint16_t cs_id)
        {
            if (chunks_.size() <= cs_id) {
                size_t n = chunks_.size();
                chunks_.resize(cs_id + 1);
                for (size_t i = n; i < chunks_.size(); ++i) {
                    chunks_[i].cs_id(i);
                }
            }
            return chunks_[cs_id];
        }

        void RtmpMessageOneContext::stream_begin(
            boost::uint32_t i)
        {
            stream_status(i, RtmpStream::started);
        }

        void RtmpMessageOneContext::stream_end(
            boost::uint32_t i)
        {
            stream_status(i, RtmpStream::eof);
        }

        void RtmpMessageOneContext::stream_status(
            boost::uint32_t i, 
            RtmpStream::StatusEnum s)
        {
            if (streams_.size() <= i) {
                streams_.resize(i + 1);
            }
            streams_[i].status = s;
        }

        void RtmpMessageOneContext::stream_is_record(
            boost::uint32_t i, 
            bool b)
        {
            if (streams_.size() <= i) {
                streams_.resize(i + 1);
            }
            streams_[i].is_record = b;
        }

        RtmpStream::StatusEnum RtmpMessageOneContext::stream_status(
            boost::uint32_t i) const
        {
            if (streams_.size() <= i) {
                return RtmpStream::stopped;
            }
            return streams_[i].status;
        }

        bool RtmpMessageOneContext::stream_is_record(
            boost::uint32_t i) const
        {
            if (streams_.size() <= i) {
                return false;
            }
            return streams_[i].is_record;
        }

        void RtmpMessageWriteContext::to_chunk(
            RtmpMessageHeader const & msg, 
            RtmpChunkHeader & chunk)
        {
            RtmpChunkHeader & wchunk(RtmpMessageOneContext::chunk(msg.chunk));
            chunk.calc_timestamp = msg.timestamp;
            chunk.message_length = msg.length;
            chunk.message_type_id = msg.type;
            chunk.message_stream_id = msg.stream;
            chunk = wchunk.dec(chunk);
            seq_ += msg.length;
        }

        void RtmpMessageReadContext::from_chunk(
            RtmpMessageHeader & msg, 
            RtmpChunkHeader const & chunk)
        {
            msg.chunk = chunk.cs_id();
            RtmpChunkHeader & rchunk(RtmpMessageOneContext::chunk(msg.chunk));
            RtmpChunkHeader const & chunk2 = 
                rchunk.add(chunk);
            msg.timestamp = chunk2.calc_timestamp;
            msg.type = chunk2.message_type_id;
            msg.length = chunk2.message_length;
            msg.stream = chunk2.message_stream_id;
            seq_ += msg.length;
        }

    } // namespace protocol
} // namespace util
