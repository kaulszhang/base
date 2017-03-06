// TcpSocket.cpp

#include "framework/Framework.h"
#include "framework/network/TcpSocket.h"

namespace framework
{
    namespace network
    {

        boost::asio::detail::mutex TcpSocket::mutex_;

    } // namespace network
} // namespace framework
