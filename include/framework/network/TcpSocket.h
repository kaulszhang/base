// TcpSocket.h

#ifndef _FRAMEWORK_NETWORK_TCP_SOCKET_H_
#define _FRAMEWORK_NETWORK_TCP_SOCKET_H_

#include "framework/network/NetName.h"
#include "framework/network/Connector.h"
#include "framework/network/Acceptor.h"

#include <boost/asio/ip/tcp.hpp>

namespace framework
{
    namespace network
    {

        class TcpSocket
            : public boost::asio::ip::tcp::socket
        {
        public:
            typedef boost::asio::ip::tcp::socket super;

        public:
            TcpSocket(
                boost::asio::io_service & io_svc)
                : super(io_svc)
                , connector_(io_svc, mutex_)
            {
            }

        public:
            boost::system::error_code set_non_block(
                bool non_block, 
                boost::system::error_code & ec)
            {
                ec = boost::system::error_code();
                boost::asio::socket_base::non_blocking_io cmd(non_block);
                {
                    boost::asio::detail::mutex::scoped_lock lock(mutex_);
                    if (is_open() && io_control(cmd, ec)) {
                        return ec;
                    }
                }
                connector_.set_non_block(non_block);
                return ec;
            }

            boost::system::error_code set_time_out(
                boost::uint32_t time_out, 
                boost::system::error_code & ec)
            {
                ec = boost::system::error_code();
                //boost::asio::socket_base::non_blocking_io cmd(time_out);
                //{
                //    boost::asio::detail::mutex::scoped_lock lock(mutex_);
                //    if (is_open() && io_control(cmd, ec)) {
                //        return ec;
                //    }
                //}
                connector_.set_time_out(time_out);
                return ec;
            }

            bool get_non_block(
                boost::system::error_code & ec) const
            {
                ec.clear();
                return connector_.get_non_block();
            }

            boost::uint32_t get_time_out(
                boost::system::error_code & ec) const
            {
                ec.clear();
                return connector_.get_time_out();
            }

        public:
            // connect
            typedef framework::network::Connector<InternetProtocol<boost::asio::ip::tcp, boost::asio::ip::tcp::socket>, boost::asio::ip::tcp::socket> connector_type;

            typedef connector_type::Statistics Statistics;

            using super::connect;

            void connect(
                framework::network::NetName const & addr)
            {
                return connector_.connect(*this, 
                    framework::network::NetName(framework::network::NetName::tcp, addr));
            }

            void get_resolvered_host_list(std::string& hosts)
            {
                connector_.get_resolvered_host_list(hosts);
            }


            boost::system::error_code connect(
                framework::network::NetName const & addr, 
                boost::system::error_code & ec)
            {
                return connector_.connect(*this, 
                    framework::network::NetName(framework::network::NetName::tcp, addr), ec);
            }

            Statistics const & stat() const
            {
                return connector_.stat();
            }

            using super::async_connect;

            template <typename Handler>
            void async_connect(
                framework::network::NetName const & addr, 
                Handler const & handler)
            {
                connector_.async_connect(*this, 
                    framework::network::NetName(framework::network::NetName::tcp, addr), handler);
            }

        public:
            // accept

            void accept(
                framework::network::NetName const & addr, 
                boost::asio::ip::tcp::acceptor & acceptor)
            {
                boost::asio::ip::address address = boost::asio::ip::address::from_string(addr.host());
                boost::asio::ip::tcp::endpoint ep(address, addr.port());
                framework::network::accept<boost::asio::ip::tcp>(acceptor, ep, *this);
            }

            boost::system::error_code accept(
                framework::network::NetName const & addr, 
                boost::asio::ip::tcp::acceptor & acceptor, 
                boost::system::error_code & ec)
            {
                boost::asio::ip::address address = 
                    boost::asio::ip::address::from_string(addr.host(), ec);
                if (ec)
                    return ec;
                boost::asio::ip::tcp::endpoint ep(address, addr.port());
                framework::network::accept<boost::asio::ip::tcp>(acceptor, ep, *this, ec);
                return ec;
            }

            void accept(
                boost::asio::ip::tcp::endpoint const & ep, 
                boost::asio::ip::tcp::acceptor & acceptor)
            {
                framework::network::accept<boost::asio::ip::tcp>(acceptor, ep, *this);
            }

            boost::system::error_code accept(
                boost::asio::ip::tcp::endpoint const & ep, 
                boost::asio::ip::tcp::acceptor & acceptor, 
                boost::system::error_code & ec)
            {
                return framework::network::accept<boost::asio::ip::tcp>(acceptor, ep, *this, ec);
            }

            template <typename Handler>
            void async_accept(
                framework::network::NetName const & addr, 
                boost::asio::ip::tcp::acceptor & acceptor, 
                Handler const & handler)
            {
                try {
                    boost::asio::ip::address address = boost::asio::ip::address::from_string(addr.host());
                    boost::asio::ip::tcp::endpoint ep(address, addr.port());
                    framework::network::async_accept<boost::asio::ip::tcp>(acceptor, ep, *this, handler);
                } catch (boost::system::system_error const & err) {
                    get_io_service().post(boost::asio::detail::bind_handler(handler, err.code()));
                }
            }

            template <typename Handler>
            void async_accept(
                boost::asio::ip::tcp::endpoint const & ep, 
                boost::asio::ip::tcp::acceptor & acceptor, 
                Handler const & handler)
            {
                framework::network::async_accept<boost::asio::ip::tcp>(acceptor, ep, *this, handler);
            }

        public:
            void cancel()
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.cancel();
                // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
                super::cancel();
#ifndef BOOST_WINDOWS_API
                // linux 需要shutdown套接字才能取消同步阻塞操作
                super::shutdown(boost::asio::socket_base::shutdown_both);
#else           // win32 shutdown套接字会发生错误
                super::close();
#endif
            }

            boost::system::error_code cancel(
                boost::system::error_code & ec)
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.cancel(ec);
                // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
                super::cancel(ec);
#ifndef BOOST_WINDOWS_API
                // linux 需要shutdown套接字才能取消同步阻塞操作
                super::shutdown(boost::asio::socket_base::shutdown_both, ec);
#else           // win32 shutdown套接字会发生错误
                super::close(ec);
#endif
                return ec;
            }

            void cancel_forever()
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.cancel_forever();
                // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
                super::cancel();
#ifndef BOOST_WINDOWS_API
                // linux 需要shutdown套接字才能取消同步阻塞操作
                super::shutdown(boost::asio::socket_base::shutdown_both);
#else           // win32 shutdown套接字会发生错误
                super::close();
#endif
            }

            boost::system::error_code cancel_forever(
                boost::system::error_code & ec)
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.cancel_forever(ec);
                // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
                super::cancel(ec);
#ifndef BOOST_WINDOWS_API
                // linux 需要shutdown套接字才能取消同步阻塞操作
                super::shutdown(boost::asio::socket_base::shutdown_both, ec);
#else           // win32 shutdown套接字会发生错误
                super::close(ec);
#endif
                return ec;
            }

            void close_forever()
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.close();
#ifndef BOOST_WINDOWS_API
                // linux 需要shutdown套接字才能取消同步阻塞操作
                super::shutdown(boost::asio::socket_base::shutdown_both);
#else           // win32 shutdown套接字会发生错误
                super::close();
#endif
            }

            void close()
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.close();
                super::close();
            }

            boost::system::error_code close(
                boost::system::error_code & ec)
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.close(ec);
                return super::close(ec);
            }

        protected:
            static boost::asio::detail::mutex mutex_;

        private:
            connector_type connector_;
        };

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_TCP_SOCKET_H_
