// Source.h

#ifndef _UTIL_STREAM_SOURCE_H_
#define _UTIL_STREAM_SOURCE_H_

#include "util/stream/Base.h"
#include "util/stream/StreamBuffers.h"
#include "util/stream/StreamHandler.h"

#include <boost/asio/detail/throw_error.hpp>

namespace util
{
    namespace stream
    {

        class Source
            : virtual public Base
        {
        public:
            typedef StreamMutableBuffers buffers_t;

            typedef StreamHandler handler_t;

        public:
            Source(
                boost::asio::io_service & io_svc)
            {
                set_io_service(io_svc);
            }

            /// Read some data from the stream. Returns the number of bytes read. Throws
            /// an exception on failure.
            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers)
            {
                boost::system::error_code ec;
                std::size_t bytes_transferred = 
                    private_read_some(buffers_t(buffers), ec);
                boost::asio::detail::throw_error(ec);
                return bytes_transferred;
            }

            /// Read some data from the stream. Returns the number of bytes read or 0 if
            /// an error occurred.
            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers,
                boost::system::error_code& ec)
            {
                std::size_t bytes_transferred = 
                    private_read_some(buffers_t(buffers), ec);
                return bytes_transferred;
            }

            /// Start an asynchronous read. The buffer into which the data will be read
            /// must be valid for the lifetime of the asynchronous operation.
            template <typename MutableBufferSequence, typename ReadHandler>
            void async_read_some(const MutableBufferSequence& buffers,
                ReadHandler handler)
            {
                private_async_read_some(
                    buffers_t(buffers), 
                    handler_t(handler));
            }

        protected:
            virtual std::size_t private_read_some(
                boost::asio::mutable_buffer const & buffer,
                boost::system::error_code & ec);

            virtual std::size_t private_read_some(
                buffers_t const & buffers,
                boost::system::error_code & ec);

            virtual void private_async_read_some(
                boost::asio::mutable_buffer const & buffer, 
                handler_t const & handler);

            virtual void private_async_read_some(
                buffers_t const & buffers, 
                handler_t const & handler);
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_SOURCE_H_
