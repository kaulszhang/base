// Socket.h

#ifndef _UTIL_STREAM_SOCKET_H_
#define _UTIL_STREAM_SOCKET_H_

#include "util/stream/Dual.h"

namespace util
{
    namespace stream
    {

        template <typename SocketType>
        class Socket
            : public Dual
        {
        public:
            Socket(
                SocketType & socket)
                : Dual(socket.get_io_service())
                , socket_(socket)
            {
            }

        private:
            virtual std::size_t private_read_some(
                StreamMutableBuffers const & buffers,
                boost::system::error_code & ec)
            {
                return socket_.read_some(buffers, ec);
            }

            virtual void private_async_read_some(
                StreamMutableBuffers const & buffers, 
                StreamHandler const & handler)
            {
                socket_.async_read_some(buffers, handler);
            }

            virtual std::size_t private_write_some(
                StreamConstBuffers const & buffers, 
                boost::system::error_code & ec)
            {
                return socket_.write_some(buffers, ec);
            }

            virtual void private_async_write_some(
                StreamConstBuffers const & buffers, 
                StreamHandler const & handler)
            {
                socket_.async_write_some(buffers, handler);
            }

        private:
            SocketType & socket_;
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_SOCKET_H_
