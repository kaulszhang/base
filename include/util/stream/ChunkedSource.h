// ChunkedSource.h

#ifndef _UTIL_STREAM_CHUNKED_SINK_H_
#define _UTIL_STREAM_CHUNKED_SINK_H_

#include "util/stream/Source.h"

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace stream
    {

        class ChunkedSource
            : public Source
        {
        public:
            ChunkedSource(
                Source & source)
                : Source(source.get_io_service())
                , source_(source)
                , rcv_left_(0)
            {
            }

        public:
            void close()
            {
                rcv_left_ = 0;
                rcv_buf_.reset();
            }

        public:
            std::size_t read_eof(
                boost::system::error_code & ec);

            template <typename WriteHandler>
            void async_read_eof(
                WriteHandler handler);

        private:
            struct recv_handler;
            friend struct recv_handler;

            static bool recv_crlf(
                boost::asio::streambuf & buf, 
                std::size_t & len);

        private:
            virtual std::size_t private_read_some(
                StreamMutableBuffers const & buffers, 
                boost::system::error_code & ec);

            virtual void private_async_read_some(
                StreamMutableBuffers const & buffers, 
                StreamHandler const & handler);

        private:
            Source & source_;
            size_t rcv_left_;
            boost::asio::streambuf rcv_buf_;
        };

        template <typename WriteHandler>
        void ChunkedSource::async_read_eof(
            WriteHandler handler)
        {
            assert(rcv_left_ == 0);
            boost::asio::async_read(source_, rcv_buf_, 
                handler);
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_STREAM_CHUNKED_H_
