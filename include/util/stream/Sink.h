// Sink.h

#ifndef _UTIL_STREAM_SINK_H_
#define _UTIL_STREAM_SINK_H_

#include "util/stream/Base.h"
#include "util/stream/StreamBuffers.h"
#include "util/stream/StreamHandler.h"

#include <boost/asio/detail/throw_error.hpp>

namespace util
{
    namespace stream
    {

        class Sink
            : virtual public Base
        {
        public:
            typedef StreamConstBuffers buffers_t;

            typedef StreamHandler handler_t;

        public:
            Sink(
                boost::asio::io_service & io_svc)
            {
                set_io_service(io_svc);
            }

            /// Read some data from the stream. Returns the number of bytes write. Throws
            /// an exception on failure.
            template <typename ConstBufferSequence>
            std::size_t write_some(
                const ConstBufferSequence& buffers)
            {
                boost::system::error_code ec;
                std::size_t bytes_transferred = 
                    private_write_some(buffers_t(buffers), ec);
                boost::asio::detail::throw_error(ec);
                return bytes_transferred;
            }

            /// Read some data from the stream. Returns the number of bytes write or 0 if
            /// an error occurred.
            template <typename ConstBufferSequence>
            std::size_t write_some(
                const ConstBufferSequence& buffers,
                boost::system::error_code& ec)
            {
                std::size_t bytes_transferred = 
                    private_write_some(buffers_t(buffers), ec);
                return bytes_transferred;
            }

            /// Start an asynchronous write. The buffer into which the data will be write
            /// must be valid for the lifetime of the asynchronous operation.
            template <typename ConstBufferSequence, typename WriteHandler>
            void async_write_some(
                const ConstBufferSequence& buffers,
                WriteHandler handler)
            {
                private_async_write_some(
                    buffers_t(buffers), 
                    handler_t(handler));
            }

        protected:
            virtual std::size_t private_write_some(
                boost::asio::const_buffer const & buffer,
                boost::system::error_code & ec);

            virtual std::size_t private_write_some(
                buffers_t const & buffers,
                boost::system::error_code & ec);

            virtual void private_async_write_some(
                boost::asio::const_buffer const & buffer, 
                handler_t const & handler);

            virtual void private_async_write_some(
                buffers_t const & buffers, 
                handler_t const & handler);
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_SINK_H_
