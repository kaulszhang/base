// Acceptor.h

#ifndef _FRAMEWORK_NETWORK_ACCEPTOR_H_
#define _FRAMEWORK_NETWORK_ACCEPTOR_H_

#include "framework/network/AsioHandlerHelper.h"

#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/asio/error.hpp>

namespace framework
{
    namespace network
    {

        template <typename InternetProtocol, typename SocketType>
        boost::system::error_code acceptor_open(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            boost::system::error_code & ec)
        {
            if (!a.is_open()) {
                if (a.open(e.protocol(), ec))
                    return ec;
				/* 端口自增不需要 reuse 模式  */
                /*{
                    boost::system::error_code ec1;
                    boost::asio::socket_base::reuse_address cmd(true);
                    a.set_option(cmd, ec1);
                }*/
                if (a.bind(e, ec))
                    return ec;
                if (a.listen(boost::asio::socket_base::max_connections,ec))
                    return ec;
            }
            ec.clear();
            return ec;
        }

        template <typename InternetProtocol, typename SocketType>
        boost::system::error_code accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            SocketType & s, // 外部创建的套接字，不需要open
            boost::system::error_code & ec)
        {
            if (acceptor_open<InternetProtocol, SocketType>(a, e, ec)) {
                return ec;
            }
            while (a.accept(s, ec) == boost::asio::error::connection_aborted);
            return ec;
        }

        template <typename InternetProtocol, typename SocketType>
        void accept(
            typename InternetProtocol::acceptor & a, 
            typename InternetProtocol::endpoint const & e, 
            SocketType & s) // 外部创建的套接字，不需要open
        {
            boost::system::error_code ec;
            accept<InternetProtocol, SocketType>(a, e, s, ec);
            boost::asio::detail::throw_error(ec);
        }
/*
        template <typename InternetProtocol, typename MutableBuffer>
        boost::system::error_code accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s, // 外部创建的套接字，不需要open
            MutableBuffer & b, // 接收缓冲区，必须由外部提供，socks可能会多接收数据
            boost::system::error_code & ec)
        {
            return accept<InternetProtocol>(a, e, s, ec);
        }

        template <typename InternetProtocol, typename MutableBuffer>
        void accept(
            typename InternetProtocol::acceptor & a, 
            typename InternetProtocol::endpoint const & e, 
            typename InternetProtocol::socket & s,  // 外部创建的套接字，不需要open
            MutableBuffer & b) // 接收缓冲区，必须由外部提供，socks可能会多接收数据
        {
            accept<InternetProtocol>(a, e, s);
        }
*/
        namespace detail
        {

            template <typename InternetProtocol, typename SocketType, typename AcceptHandler>
            class accept_handler
            {
            public:
                /// The network type.
                typedef InternetProtocol protocol_type;

                /// The acceptor type.
                typedef typename InternetProtocol::acceptor acceptor;

                /// The socket type.
                typedef SocketType socket;

                accept_handler(
                    acceptor & a, 
                    socket & s, 
                    AcceptHandler handler)
                    : acceptor_(a)
                    , socket_(s)
                    , handler_(handler)
                {
                }

            public:
                void operator ()(
                    boost::system::error_code const & ec)
                {
                    if (ec == boost::asio::error::connection_aborted) {
                        acceptor_.async_accept(socket_, *this);
                        return;
                    }
                    handler_(ec);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(accept_handler, handler_)

            private:
                acceptor & acceptor_;
                socket & socket_; // TCP套接字
                AcceptHandler handler_;
            };

        } // namespace detail

        template <typename InternetProtocol, typename SocketType, typename AcceptHandler>
        void async_accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            SocketType & s, // 外部创建的套接字，不需要open
            AcceptHandler const & handler)
        {
            if (!a.is_open()) {
                boost::system::error_code ec;
                if (acceptor_open<InternetProtocol, SocketType>(a, e, ec)) {
                    a.get_io_service().post(
                        boost::asio::detail::bind_handler(handler, ec));
                    return;
                }
            }
            a.async_accept(s, 
                detail::accept_handler<InternetProtocol, SocketType, AcceptHandler>(a, s, handler));
        }
/*
        template <typename InternetProtocol, typename MutableBuffer, typename AcceptHandler>
        void async_accept(
            typename InternetProtocol::acceptor & a,
            typename InternetProtocol::endpoint const & e,
            typename InternetProtocol::socket & s, // 外部创建的套接字，不需要open
            MutableBuffer & b, // 接收缓冲区，必须由外部提供，socks可能会多接收数据
            AcceptHandler const & handler)
        {
            async_accept(a, e, s, handler);
        }
*/
    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_ACCEPTOR_H_
