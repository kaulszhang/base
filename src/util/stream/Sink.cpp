// Sink.cpp

#include "util/Util.h"
#include "util/stream/Sink.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/bind_handler.hpp>

namespace util
{
    namespace stream
    {

        std::size_t Sink::private_write_some(
            boost::asio::const_buffer const & buffer,
            boost::system::error_code & ec)
        {
            return private_write_some(
                StreamConstBuffers(boost::asio::const_buffers_1(buffer)), ec);
        }

        std::size_t Sink::private_write_some(
            StreamConstBuffers const & buffers,
            boost::system::error_code & ec)
        {
            using namespace boost::asio;

            typedef StreamConstBuffers::const_iterator const_iterator;
            std::size_t bytes_transferred = 0;
            for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
                bytes_transferred += private_write_some(*iter, ec);
                if (ec) break;
            }
            return bytes_transferred;
        }

        void Sink::private_async_write_some(
            boost::asio::const_buffer const & buffer, 
            StreamHandler const & handler)
        {
            boost::system::error_code ec;
            std::size_t bytes_transferred = private_write_some(buffer, ec);
            get_io_service().post(
                boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
        }

        void Sink::private_async_write_some(
            StreamConstBuffers const & buffers, 
            StreamHandler const & handler)
        {
            boost::system::error_code ec;
            std::size_t bytes_transferred = private_write_some(buffers, ec);
            get_io_service().post(
                boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
        }

    } // namespace stream
} // namespace util
