// TcpsSocket.h

#ifndef _FRAMEWORK_NETWORK_TCPS_SOCKET_H_
#define _FRAMEWORK_NETWORK_TCPS_SOCKET_H_

#include "framework/network/NetName.h"
#include "framework/network/Connector.h"
#include "framework/network/Acceptor.h"

#include <boost/function.hpp>
#include <boost/asio/ip/tcp.hpp>

#ifdef ENABLE_OPENSSL
#include <boost/asio/ssl.hpp>
#endif

namespace framework
{
    namespace network
    {
#ifdef ENABLE_OPENSSL
        class TcpsSocket
        {
        public:

            typedef boost::function<void(
                boost::system::error_code const &)
            > response_type;

            typedef TcpsSocket super;

            TcpsSocket(
                boost::asio::io_service & io_svc)
                : io_svc_(io_svc)
                , resolver_(io_svc)
                , socket_(NULL)
                , connector_(io_svc, mutex_)
            {
                boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);
                context.set_default_verify_paths();
                socket_ = new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(io_svc, context);
                socket_->set_verify_mode(boost::asio::ssl::context::verify_peer);
                socket_->set_verify_callback(boost::bind(&TcpsSocket::verify_certificate, this, _1, _2));
            }

            virtual ~TcpsSocket()
            {
                if (socket_)
                {
                    delete socket_;
                }
                socket_ = NULL;
            }

        public:

            bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)
            {
                char subject_name[256];
                X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
                X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
                std::cout << "Verifying:\n" << subject_name << std::endl;
                return true;
            }

            boost::system::error_code set_non_block(
                bool non_block,
                boost::system::error_code & ec)
            {
                ec = boost::system::error_code();
                boost::asio::socket_base::non_blocking_io cmd(non_block);
                {
                    boost::asio::detail::mutex::scoped_lock lock(mutex_);
                    if (socket_->lowest_layer().is_open() && socket_->lowest_layer().io_control(cmd, ec)) {
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

            /*template <typename MutableBufferSequence, typename ReadHandler>
            void async_receive(
            const MutableBufferSequence& buffers,
            boost::asio::socket_base::message_flags flags,
            ReadHandler handler)
            {
            socket_->async_receive(buffers, flags, handler);
            }

            template <typename MutableBufferSequence>
            std::size_t receive(const MutableBufferSequence& buffers,
            boost::asio::socket_base::message_flags flags)
            {
            return socket_->receive(buffers, flags);
            }

            template <typename MutableBufferSequence>
            std::size_t receive(
            const MutableBufferSequence& buffers,
            boost::asio::socket_base::message_flags flags,
            boost::system::error_code& ec)
            {
            return socket_->receive(buffers, flags, ec);
            }*/

            template <typename MutableBufferSequence>
            std::size_t read_some(const MutableBufferSequence& buffers,
                boost::system::error_code& ec)
            {
                return socket_->read_some(buffers, ec);
            }

            template <typename MutableBufferSequence>
            std::size_t read_some(const MutableBufferSequence& buffers)
            {
                //与上面相同，但不带ec
                return socket_->read_some(buffers);
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_read_some(const MutableBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(ReadHandler) handler)
            {
                socket_->async_read_some(buffers, handler);
            }

            template <typename ConstBufferSequence>
            std::size_t write_some(const ConstBufferSequence& buffers, boost::system::error_code& ec)
            {
                return socket_->write_some(buffers, ec);
            }

            template <typename ConstBufferSequence>
            std::size_t write_some(const ConstBufferSequence& buffers)
            {
                //与上面相同，但不带ec
                return socket_->write_some(buffers);
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_write_some(const MutableBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(ReadHandler) handler)
            {
                socket_->async_write_some(buffers, handler);
            }

        public:
            // connect
            typedef framework::network::Connector<InternetProtocol<boost::asio::ip::tcp, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type>, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type> connector_type;

            typedef connector_type::Statistics Statistics;


            void get_resolvered_host_list(std::string& hosts)
            {
                connector_.get_resolvered_host_list(hosts);
            }

            void connect(
                framework::network::NetName const & addr)
            {
                connector_.connect(socket_->lowest_layer(),
                    framework::network::NetName(framework::network::NetName::tcp, addr));
                socket_->handshake(boost::asio::ssl::stream_base::client);
            }

            boost::system::error_code connect(
                framework::network::NetName const & addr,
                boost::system::error_code & ec)
            {
                connector_.connect(socket_->lowest_layer(),
                    framework::network::NetName(framework::network::NetName::tcp, addr), ec);
                return ((ec) ? ec : socket_->handshake(boost::asio::ssl::stream_base::client, ec));
            }

            boost::asio::io_service &get_io_service()
            {
                return io_svc_;
            }

            Statistics const & stat() const
            {
                return connector_.stat();
            }

            //using super::async_connect;

            //template <typename Handler>
            void async_connect(
                framework::network::NetName const & addr,
                response_type const & resp)
            {
                connector_.async_connect(socket_->lowest_layer(),
                    framework::network::NetName(framework::network::NetName::tcp, addr), 
                    boost::bind(&TcpsSocket::handle_connect, this, resp, boost::asio::placeholders::error));
            }
        private:
            void handle_connect(response_type const & resp, const boost::system::error_code& ec)
            {
                if (ec)
                    resp(ec);
                else
                    socket_->async_handshake(boost::asio::ssl::stream_base::client, 
                    boost::bind(&TcpsSocket::handle_handshake, this, resp,  boost::asio::placeholders::error));
            }

            void handle_handshake(response_type const & resp, const boost::system::error_code& ec)
            {
                resp(ec);
            }

        public:
            void accept(
                framework::network::NetName const & addr,
                boost::asio::ip::tcp::acceptor & acceptor)
            {
                boost::asio::ip::address address = boost::asio::ip::address::from_string(addr.host());
                boost::asio::ip::tcp::endpoint ep(address, addr.port());
                framework::network::accept<boost::asio::ip::tcp, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type>(acceptor, ep, socket_->lowest_layer());
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
                framework::network::accept<boost::asio::ip::tcp, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type>(acceptor, ep, socket_->lowest_layer(), ec);
                return ec;
            }

            void accept(
                boost::asio::ip::tcp::endpoint const & ep,
                boost::asio::ip::tcp::acceptor & acceptor)
            {
                framework::network::accept<boost::asio::ip::tcp, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type>(acceptor, ep, socket_->lowest_layer());
            }

            boost::system::error_code accept(
                boost::asio::ip::tcp::endpoint const & ep,
                boost::asio::ip::tcp::acceptor & acceptor,
                boost::system::error_code & ec)
            {
                return framework::network::accept<boost::asio::ip::tcp, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type>(acceptor, ep, socket_->lowest_layer(), ec);
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
                    framework::network::async_accept<boost::asio::ip::tcp>(acceptor, ep, socket_->lowest_layer(), handler);
                }
                catch (boost::system::system_error const & err) {
                    get_io_service().post(boost::asio::detail::bind_handler(handler, err.code()));
                }
            }

            template <typename Handler>
            void async_accept(
                boost::asio::ip::tcp::endpoint const & ep,
                boost::asio::ip::tcp::acceptor & acceptor,
                Handler const & handler)
            {
                framework::network::async_accept<boost::asio::ip::tcp>(acceptor, ep, socket_->lowest_layer(), handler);
            }

        public:

            boost::system::error_code shutdown(boost::asio::socket_base::shutdown_type what,
                boost::system::error_code& ec)
            {
                return socket_->lowest_layer().shutdown(what, ec);
            }

            void cancel()
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.cancel();

                // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
                socket_->lowest_layer().cancel();
#ifndef BOOST_WINDOWS_API
                // linux 需要shutdown套接字才能取消同步阻塞操作
                socket_->lowest_layer().shutdown(boost::asio::socket_base::shutdown_both);
#else           // win32 shutdown套接字会发生错误
                socket_->lowest_layer().close();
#endif
            }

            boost::system::error_code cancel(
                boost::system::error_code & ec)
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.cancel(ec);
                // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
                socket_->lowest_layer().cancel(ec);
#ifndef BOOST_WINDOWS_API
                // linux 需要shutdown套接字才能取消同步阻塞操作
                socket_->lowest_layer().shutdown(boost::asio::socket_base::shutdown_both, ec);
#else           // win32 shutdown套接字会发生错误
                socket_->lowest_layer().close(ec);
#endif
                return ec;
            }

            void cancel_forever()
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.cancel_forever();
                // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
                socket_->lowest_layer().cancel();
#ifndef BOOST_WINDOWS_API
                // linux 需要shutdown套接字才能取消同步阻塞操作
                socket_->lowest_layer().shutdown(boost::asio::socket_base::shutdown_both);
#else           // win32 shutdown套接字会发生错误
                socket_->lowest_layer().close();
#endif
            }

            boost::system::error_code cancel_forever(
                boost::system::error_code & ec)
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.cancel_forever(ec);
                // 取消异步操作，在某些平台仅仅close或者shutdown不能取消异步操作
                socket_->lowest_layer().cancel(ec);
#ifndef BOOST_WINDOWS_API
                // linux 需要shutdown套接字才能取消同步阻塞操作
                socket_->lowest_layer().shutdown(boost::asio::socket_base::shutdown_both, ec);
#else           // win32 shutdown套接字会发生错误
                socket_->lowest_layer().close(ec);
#endif
                return ec;
            }

            void close_forever()
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.close();
#ifndef BOOST_WINDOWS_API
                // linux 需要shutdown套接字才能取消同步阻塞操作
                socket_->lowest_layer().shutdown(boost::asio::socket_base::shutdown_both);
#else           // win32 shutdown套接字会发生错误
                socket_->lowest_layer().close();
#endif
            }

            void close()
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.close();
                socket_->lowest_layer().close();
            }

            boost::system::error_code close(
                boost::system::error_code & ec)
            {
                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                connector_.close(ec);
                return socket_->lowest_layer().close(ec);
            }

        protected:
            static boost::asio::detail::mutex mutex_;

        private:
            boost::asio::io_service & io_svc_;
            boost::asio::ip::tcp::resolver resolver_;
            boost::asio::ssl::stream<boost::asio::ip::tcp::socket>* socket_;
            connector_type connector_;
        };
#else
        class TcpsSocket
            : public boost::asio::ip::tcp::socket
        {
        public:
            typedef boost::asio::ip::tcp::socket super;

        public:
            TcpsSocket(
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
                }
                catch (boost::system::system_error const & err) {
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

#endif

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_TCP_SOCKET_H_
