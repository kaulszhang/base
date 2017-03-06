// TcpSocket.h

#ifndef _UTIL_STREAM_TCP_SOCKET_H_
#define _UTIL_STREAM_TCP_SOCKET_H_

#include "util/stream/Socket.h"

#include <boost/asio/ip/tcp.hpp>

namespace util
{
    namespace stream
    {

        typedef Socket<boost::asio::ip::tcp::socket> TcpSocket;

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_TCP_SOCKET_H_
