// StlStream.h

#ifndef _UTIL_STREAM_STL_STREAM_H_
#define _UTIL_STREAM_STL_STREAM_H_

#include <framework/system/ErrorCode.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/bind_handler.hpp>

#include <istream>
#include <ostream>

namespace util
{
    namespace stream
    {

        class StlIStream
        {
        public:
            StlIStream(
                boost::asio::io_service & io_svc, 
                std::istream & impl)
                : io_svc_(io_svc)
                , impl_(impl)
            {
            }

            /// Close the stream.
            void close()
            {
            }

            /// Read some data from the stream. Returns the number of bytes read. Throws
            /// an exception on failure.
            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers)
            {
                boost::system::error_code ec;
                std::size_t bytes_transferred = read_some(buffers, ec);
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
                using namespace boost::asio;

                typedef typename MutableBufferSequence::const_iterator const_iterator;
                std::size_t total_transferred = 0;
                for (const_iterator iter = buffers.begin(); !!impl_ && iter != buffers.end(); ++iter) {
                    boost::asio::mutable_buffer buffer(*iter);
                    impl_.read(buffer_cast<char *>(buffer), buffer_size(buffer));
                    total_transferred += impl_.gcount();
                }
                if (!!impl_) {
                    ec = boost::system::error_code(framework::system::last_system_error().value(), boost::system::system_category());
                }
                return total_transferred;
            }

            /// Start an asynchronous read. The buffer into which the data will be read
            /// must be valid for the lifetime of the asynchronous operation.
            template <typename MutableBufferSequence, typename ReadHandler>
            void async_read_some(const MutableBufferSequence& buffers,
                ReadHandler handler)
            {
                boost::system::error_code ec;
                std::size_t bytes_transferred = read_some(buffers, ec);
                io_svc_.post(boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
            }

            boost::asio::io_service & get_io_service()
            {
                return io_svc_;
            }

        private:
            boost::asio::io_service & io_svc_;
            std::istream & impl_;
        };

        class StlOStream
        {
        public:
            StlOStream(
                boost::asio::io_service & io_svc, 
                std::ostream & impl)
                : io_svc_(io_svc)
                , impl_(impl)
            {
            }

            /// Close the stream.
            void close()
            {
            }

            /// Read some data from the stream. Returns the number of bytes read. Throws
            /// an exception on failure.
            template <typename ConstBufferSequence>
            std::size_t write_some(
                const ConstBufferSequence & buffers)
            {
                boost::system::error_code ec;
                std::size_t bytes_transferred = write_some(buffers, ec);
                boost::asio::detail::throw_error(ec);
                return bytes_transferred;
            }

            /// Read some data from the stream. Returns the number of bytes read or 0 if
            /// an error occurred.
            template <typename ConstBufferSequence>
            std::size_t write_some(
                const ConstBufferSequence & buffers,
                boost::system::error_code & ec)
            {
                using namespace boost::asio;

                typedef typename ConstBufferSequence::const_iterator const_iterator;
                std::size_t total_transferred = 0;
                if (impl_.good()) {
                    for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
                        boost::asio::const_buffer buffer(*iter);
                        impl_.write(buffer_cast<char const *>(buffer), buffer_size(buffer));
                        if (impl_.good())
                            total_transferred += buffer_size(buffer);
                        else
                            break;
                    }
                }
                if (!impl_.good()) {
                    ec = boost::system::error_code(framework::system::last_system_error().value(), boost::system::system_category());
                }
                return total_transferred;
            }

            /// Start an asynchronous read. The buffer into which the data will be read
            /// must be valid for the lifetime of the asynchronous operation.
            template <typename ConstBufferSequence, typename ReadHandler>
            void async_write_some(const ConstBufferSequence& buffers,
                ReadHandler handler)
            {
                boost::system::error_code ec;
                std::size_t bytes_transferred = write_some(buffers, ec);
                io_svc_.post(boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
            }

        private:
            boost::asio::io_service & io_svc_;
            std::ostream & impl_;
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_STL_STREAM_H_
