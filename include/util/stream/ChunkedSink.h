// ChunkedSink.h

#ifndef _UTIL_STREAM_CHUNKED_SINK_H_
#define _UTIL_STREAM_CHUNKED_SINK_H_

#include "util/stream/Sink.h"

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace stream
    {

        class ChunkedSink
            : public Sink
        {
        public:
            ChunkedSink(
                Sink & sink)
                : Sink(sink.get_io_service())
                , sink_(sink)
                , snd_left_(0)
            {
            }

        public:
            void close()
            {
                snd_left_ = 0;
                snd_buf_.reset();
            }

        public:
            std::size_t write_eof(
                boost::system::error_code & ec);

            template <typename WriteHandler>
            void async_write_eof(
                WriteHandler handler);

        private:
            struct send_handler;
            friend struct send_handler;

            static void make_chunk_head(
                boost::asio::streambuf & buf, 
                std::size_t len);

            static void make_chunk_tail(
                boost::asio::streambuf & buf);

            static void make_chunk_eof(
                boost::asio::streambuf & buf);

        private:
            virtual std::size_t private_write_some(
                StreamConstBuffers const & buffers, 
                boost::system::error_code & ec);

            virtual void private_async_write_some(
                StreamConstBuffers const & buffers, 
                StreamHandler const & handler);

        private:
            Sink & sink_;
            size_t snd_left_;
            boost::asio::streambuf snd_buf_;
        };

        template <typename WriteHandler>
        void ChunkedSink::async_write_eof(
            WriteHandler handler)
        {
            assert(snd_left_ == 0);
            if (snd_buf_.size() == 0) {
                make_chunk_eof(snd_buf_);
            }
            boost::asio::async_write(sink_, snd_buf_, 
                boost::asio::transfer_all(), 
                handler);
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_STREAM_CHUNKED_H_
