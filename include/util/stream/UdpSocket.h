// UdpSocket.h

#ifndef _UTIL_STREAM_UDP_SOCKET_H_
#define _UTIL_STREAM_UDP_SOCKET_H_

#include "util/stream/Socket.h"

#include <boost/asio/ip/udp.hpp>

namespace util
{
    namespace stream
    {

        typedef Socket<boost::asio::ip::udp::socket> UdpSocket;

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_UDP_SOCKET_H_
