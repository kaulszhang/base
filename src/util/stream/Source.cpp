// Source.cpp

#include "util/Util.h"
#include "util/stream/Source.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/bind_handler.hpp>

namespace util
{
    namespace stream
    {

        std::size_t Source::private_read_some(
            boost::asio::mutable_buffer const & buffer,
            boost::system::error_code & ec)
        {
            return private_read_some(
                StreamMutableBuffers(boost::asio::mutable_buffers_1(buffer)), ec);
        }

        std::size_t Source::private_read_some(
            StreamMutableBuffers const & buffers,
            boost::system::error_code & ec)
        {
            using namespace boost::asio;

            typedef StreamMutableBuffers::const_iterator const_iterator;
            std::size_t bytes_transferred = 0;
            for (const_iterator iter = buffers.begin(); iter != buffers.end(); ++iter) {
                bytes_transferred += private_read_some(*iter, ec);
                if (ec) break;
            }
            return bytes_transferred;
        }

        void Source::private_async_read_some(
            boost::asio::mutable_buffer const & buffer, 
            StreamHandler const & handler)
        {
            boost::system::error_code ec;
            std::size_t bytes_transferred = private_read_some(buffer, ec);
            get_io_service().post(
                boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
        }

        void Source::private_async_read_some(
            StreamMutableBuffers const & buffers, 
            StreamHandler const & handler)
        {
            boost::system::error_code ec;
            std::size_t bytes_transferred = private_read_some(buffers, ec);
            get_io_service().post(
                boost::asio::detail::bind_handler(handler, ec, bytes_transferred));
        }

    } // namespace stream
} // namespace util
