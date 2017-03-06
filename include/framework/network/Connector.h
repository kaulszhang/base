// Connector.h

#ifndef _FRAMEWORK_NETWORK_CONNECTOR_H_
#define _FRAMEWORK_NETWORK_CONNECTOR_H_

#include "framework/network/NetName.h"
#include "framework/network/Resolver.h"
#include "framework/network/Statistics.h"
#include "framework/logger/Logger.h"
#include "framework/logger/LoggerStreamRecord.h"
#include "framework/logger/LoggerSection.h"
#include "framework/network/AsioHandlerHelper.h"

#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/detail/socket_ops.hpp>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace framework
{
    namespace network
    {

        template <typename Protocol, typename SocketType>
        class Connector;

        template <typename _InternetProtocol, typename _SocketType>
        class InternetProtocol
        {
        public:
            typedef framework::network::NetName netname;

            typedef framework::network::Endpoint endpoint;

            typedef _SocketType socket;
            
            typedef Connector<InternetProtocol, _SocketType> connector;

            typedef typename _InternetProtocol::endpoint base_endpoint;

            typedef framework::network::Resolver resolver;

            typedef typename _InternetProtocol::resolver::query resolver_query;

            typedef typename _InternetProtocol::resolver::iterator resolver_iterator;

            static _InternetProtocol v4()
            {
                return _InternetProtocol::v4();
            }

            static _InternetProtocol v6()
            {
                return _InternetProtocol::v6();
            }
        };

        namespace detail
        {
            struct ConnectorStatistics
                : public TimeStatistics
            {
                void reset()
                {
                    TimeStatistics::reset();
                    resolve_time = connect_time = (boost::uint32_t)-1;
                    last_error.clear();
                }

                void zero()
                {
                    resolve_time = connect_time = 0;
                    last_error.clear();
                }

                boost::uint32_t resolve_time;
                boost::uint32_t connect_time; // total time
                boost::system::error_code last_error;
            };
        }

        typedef struct _tAsyncRet
        {
            _tAsyncRet()
                :count_(0)
            {
            }

            void reset()
            {
                ec_.clear();
                count_ = 0;
            }

            void set_ec(
                boost::system::error_code e)
            {
                ec_ = e;
            }

            void set_count(
                size_t c)
            {
                count_ = c;
            }

            boost::system::error_code get_ec() const
            {
                return ec_;
            }

            size_t get_count() const
            {
                return count_;
            }

            const struct _tAsyncRet operator++()
            {
                count_++;
                return *this;
            }

        private:
            boost::system::error_code ec_;
            size_t count_;
        } async_ret_type;

        template <typename Protocol, typename SocketType>
        class Connector
        {
            FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Connector", 2)

        public:
            /// The protocol type.
            typedef Protocol protocol_type;

            /// The netname type.
            typedef typename Protocol::endpoint endpoint_type;

            /// The netname type.
            typedef typename Protocol::netname netname_type;

            typedef SocketType socket_type;

            typedef typename Protocol::base_endpoint base_endpoint_type;

            //typedef typename Protocol::resolver resolver_type;
            typedef Resolver resolver_type;

            typedef typename Protocol::resolver_query resolver_query;

            // typedef typename Protocol::resolver_iterator resolver_iterator;
            typedef ResolverIterator resolver_iterator;

        public:
            Connector(
                boost::asio::io_service & io_svc, 
                boost::asio::detail::mutex & mutex)
                : non_block_(false)
                , time_out_(0)
                , started_(false)
                , connect_started_(false)
                , canceled_(false)
                , canceled_forever_(false)
                , resolver_(io_svc)
                , mutex_(mutex)
                , timer_(io_svc, boost::posix_time::milliseconds(0))
            {
            }

            void get_resolvered_host_list(std::string& hosts)
            {
                if (use_ip_.empty() || last_ip_.empty())
                    return;
                hosts = use_ip_;
                hosts += "=";
                hosts += last_ip_;
            }

            void open(
                protocol_type const &)
            {
            }

            void set_non_block(
                bool non_block)
            {
                non_block_ = non_block;
            }

            void set_time_out(
                boost::uint32_t time_out)
            {
                time_out_ = time_out;
            }

            boost::system::error_code set_non_block(
                bool non_block, 
                boost::system::error_code & ec)
            {
                non_block_ = non_block;
                return ec = boost::system::error_code();
            }

            boost::uint32_t get_time_out() const
            {
                return time_out_;
            }

            bool get_non_block() const
            {
                return non_block_;
            }

            bool is_open()
            {
                return true;
            }

            void bind(
                netname_type const & netname)
            {
            }

            boost::system::error_code bind(
                netname_type const & netname,
                boost::system::error_code & ec)
            {
                return ec = boost::system::error_code();
            }

            void start_connect(
                socket_type & peer, 
                endpoint_type const & ep, 
                boost::system::error_code & ec)
            {
                if (!non_block_ && time_out_) {
                    boost::asio::socket_base::non_blocking_io cmd(true);
                    peer.io_control(cmd, ec);
                    if (ec)
                        return;
                }
                if (!connect_started_) {
                    connect_started_ = true;
                    peer.connect(ep, ec);
                }
                if ((ec == boost::asio::error::in_progress || 
                    ec == boost::asio::error::already_started)) {
                        ec = boost::asio::error::would_block;
                }
            }

            void pool_connect(
                socket_type & peer, 
                boost::system::error_code & ec)
            {
                fd_set writefds;
                fd_set exceptfds;
                timeval timeout;
                FD_ZERO(&writefds);
                FD_ZERO(&exceptfds);
                boost::asio::detail::socket_type fd = peer.native();
                FD_SET(fd, &writefds);
                FD_SET(fd, &exceptfds);
                timeval * ptimeout = &timeout;
                if (non_block_) {
                    timeout.tv_sec = 0;
                    timeout.tv_usec = 0;
                } else if (time_out_) {
                    timeout.tv_sec = time_out_ / 1000;
                    timeout.tv_usec = (time_out_ % 1000) * 1000;
                } else {
                    ptimeout = NULL;
                }
                int ret = boost::asio::detail::socket_ops::select(
                    fd + 1, NULL, &writefds, &exceptfds, ptimeout, ec);
                if (ret > 0) {
                    boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_ERROR> err;
                    peer.get_option(err);
                    ec = boost::asio::error::basic_errors(err.value());
                } else if (ret == 0) {
                    if (non_block_) {
                        ec = boost::asio::error::would_block;
                    } else {
                        ec = boost::asio::error::timed_out;
                    }
                }
            }

            void post_connect(
                socket_type & peer, 
                boost::system::error_code & ec)
            {
                if (!non_block_ && time_out_) {
                    boost::asio::socket_base::non_blocking_io cmd(false);
                    boost::system::error_code ec1;
                    peer.io_control(cmd, ec1);
                }
                // support non-blocking mode
                if (ec == boost::asio::error::in_progress || 
                    ec == boost::asio::error::already_started || 
                    ec == boost::asio::error::would_block) {
                        ec = boost::asio::error::would_block;
                } else if (ec == boost::asio::error::already_connected) {
                    ec = boost::system::error_code();
                } else {
                    connect_started_ = false;
                }
            }

            boost::system::error_code connect(
                socket_type & peer, 
                netname_type const & netname, 
                boost::system::error_code & ec)
            {
                use_ip_ = netname.host();
                if (netname.is_digit()) {
                    last_ip_ = netname.host();
                    if (!started_) {
                        canceled_ = canceled_forever_;
                        stat_.reset();
                        stat_.resolve_time = 0;
                        endpoint_type e;
                        e.port(netname.port());
                        e.ip(netname.host());
                        if (ec) {
                            return ec;
                        }
                        {
                            boost::asio::detail::mutex::scoped_lock lock(mutex_);
                            if (canceled_) {
                                ec = boost::asio::error::operation_aborted;
                                canceled_ = false;
                            } else if (peer.is_open()) {
                                peer.close(ec);
                            }
                            boost::asio::socket_base::non_blocking_io cmd1(non_block_);
                            ec || peer.open(base_endpoint_type(e).protocol(), ec) 
                                || peer.io_control(cmd1, ec)
                                ;
                            started_ = true;
                        }
                        connect_started_ = false;
                        if (ec)
                            return ec;
                        start_connect(peer, e, ec);
                    } else {
                        ec = boost::asio::error::would_block;
                    }
                    if (ec == boost::asio::error::would_block) {
                        pool_connect(peer, ec);
                    }
                    if (ec == boost::asio::error::would_block) {
                        if (time_out_ && stat_.elapse() > time_out_) {
                            ec = boost::asio::error::timed_out;
                        }
                    }
                    if (ec != boost::asio::error::would_block) {
                        if (!ec) {
                            post_connect(peer, ec);
                        } else {
                            boost::system::error_code ec1;
                            post_connect(peer, ec1);
                        }
                        stat_.connect_time = stat_.elapse();
                        started_ = false;
                        canceled_ = false;
                    }
                } else {
                    if (!started_) {
                        canceled_ = canceled_forever_;
                        stat_.reset();
                        connect_started_ = false;
                        boost::asio::detail::mutex::scoped_lock lock(mutex_);
                        if (canceled_) {
                            ec = boost::asio::error::operation_aborted;
                            canceled_ = false;
                        } else {
                            lock.unlock();
                            resolver_iterator_ = resolver_.resolve(netname, ec);
                            lock.lock();
                        }
                        stat_.resolve_time = stat_.elapse();
                        if (ec) {
                            return ec;
                        } else if (canceled_) {
                            canceled_ = false;
                            return ec = boost::asio::error::operation_aborted;
                        }
                        started_ = true;
                    }
                    resolver_iterator end;
                    for (; resolver_iterator_ != end; ++resolver_iterator_) {
                        if (!connect_started_) {
                            endpoint_type const & e = *resolver_iterator_;
                            {
                                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                                if (canceled_) {
                                    ec = boost::asio::error::operation_aborted;
                                    canceled_ = false;
                                } else {
                                    if (peer.is_open()) {
                                        peer.close(ec);
                                    }
                                    boost::asio::socket_base::non_blocking_io cmd1(non_block_);
                                    ec || peer.open(base_endpoint_type(e).protocol(), ec) 
                                        || peer.io_control(cmd1, ec) 
                                        ;
                                }
                            }
                            if (ec) {
                                break;
                            }
                            last_ip_ = e.ip_str();
                            LOG_S(framework::logger::Logger::kLevelDebug1, "[connect] try server, ep: " << e.to_string());
                            start_connect(peer, e, ec);
                        }  else {
                            ec = boost::asio::error::would_block;
                        }
                        if (ec == boost::asio::error::would_block) {
                            pool_connect(peer, ec);
                        }
                        if (ec != boost::asio::error::would_block) {
                            if (!ec) {
                                post_connect(peer, ec);
                            } else {
                                boost::system::error_code ec1;
                                post_connect(peer, ec1);
                            }
                        }
                        if (!ec || ec == boost::asio::error::would_block || canceled_) {
                            break;
                        }
                        LOG_S(framework::logger::Logger::kLevelDebug, "[connect] failed, ep: " << 
                            resolver_iterator_->to_string() << ",ec: " << ec.message());
                    } // for
                    if ((!ec || ec == boost::asio::error::would_block) && canceled_) {
                        ec = boost::asio::error::operation_aborted;
                    }
                    if (ec != boost::asio::error::would_block) {
                        stat_.connect_time = stat_.elapse();
                        started_ = false;
                        canceled_ = false;
                    }
                }
                return ec;
            }

            void connect(
                socket_type & peer, 
                netname_type const & netname)
            {
                boost::system::error_code ec;
                connect(peer, netname, ec);
                boost::asio::detail::throw_error(ec);
            }

            template <typename ConnectHandler>
            void async_connect(
                socket_type & peer, 
                netname_type const & netname, 
                ConnectHandler const & handler);

            boost::system::error_code cancel_forever(
                boost::system::error_code & ec)
            {
                canceled_ = true;
                canceled_forever_ = true;
                resolver_.cancel(ec);
                return ec;
            }

            void cancel_forever()
            {
                canceled_ = true;
                canceled_forever_ = true;
                boost::system::error_code ec;
                cancel(ec);
                boost::asio::detail::throw_error(ec);
            }

            boost::system::error_code cancel(
                boost::system::error_code & ec)
            {
                canceled_ = true;
                resolver_.cancel(ec);
                return ec;
            }

            void cancel()
            {
                canceled_ = true;
                boost::system::error_code ec;
                cancel(ec);
                boost::asio::detail::throw_error(ec);
            }

            boost::system::error_code close(
                boost::system::error_code & ec)
            {
                started_ = false;
                ec.clear();
                canceled_ = false;
                return ec;
            }

            void close()
            {
                started_ = false;
                canceled_ = false;
            }

        public:
            typedef detail::ConnectorStatistics Statistics;

            Statistics const & stat() const
            {
                return stat_;
            }

        private:
            bool non_block_;
            boost::uint32_t time_out_; // 同步connector超时时间/ms
            bool started_;
            bool connect_started_;
            bool canceled_;
            bool canceled_forever_;
            resolver_type resolver_;
            resolver_iterator resolver_iterator_;
            boost::asio::detail::mutex & mutex_;
            Statistics stat_;

            boost::asio::deadline_timer timer_;
            async_ret_type async_ret_;
            std::string use_ip_;
            std::string last_ip_;
        };

        namespace detail
        {

            template <typename InternetProtocol, typename ConnectHandler>
            class connect_handler
            {
                FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Connector", 2)
            public:
                /// The protocol type.
                typedef InternetProtocol protocol_type;

                /// The socket type.
                typedef typename InternetProtocol::socket socket;

                /// The netname type.
                typedef typename InternetProtocol::netname netname_type;

                /// The netname type.
                typedef typename InternetProtocol::endpoint endpoint_type;

                /// The netname type.
                typedef typename InternetProtocol::base_endpoint base_endpoint_type;

                /// The resolver type.
                typedef typename InternetProtocol::resolver resolver;

                /// The query type.
                typedef typename InternetProtocol::resolver_query query;

                /// The iterator type.
                // typedef typename InternetProtocol::resolver_iterator iterator;
                typedef ResolverIterator iterator;

                typedef void result_type;

                connect_handler(
                    socket & s, 
                    bool non_block, 
                    boost::asio::detail::mutex & mutex, 
                    resolver & rsv, 
                    bool & canceled, 
                    detail::ConnectorStatistics & stat, 
                    ConnectHandler handler,
                    boost::uint32_t time_out,
                    boost::asio::deadline_timer & timer,
                    async_ret_type & async_ret,
                    std::string& use_ip)
                    : socket_(s)
                    , non_block_(non_block)
                    , mutex_(mutex)
                    , canceled_(canceled)
                    , resolver_(rsv)
                    , stat_(stat)
                    , handler_(handler)
                    , time_out_(time_out)
                    , timer_(timer)
                    , async_ret_(async_ret)
                    , last_ip_(use_ip)
                {
                    last_ip_.c_str();
                }

                void start(
                    endpoint_type const & e)
                {
                    boost::system::error_code ec;
                    last_ip_ = e.ip_str();
                    {
                        boost::asio::detail::mutex::scoped_lock lock(mutex_);
                        if (canceled_) {
                            ec = boost::asio::error::operation_aborted;
                        } else {
                            boost::asio::socket_base::non_blocking_io cmd(non_block_);
                            socket_.open(base_endpoint_type(e).protocol(), ec)
                                || socket_.io_control(cmd, ec);
                        }
                    }
                    if (ec) {
                        stat_.connect_time = stat_.elapse();
                        canceled_ = false;
                        socket_.get_io_service().post(
                            boost::asio::detail::bind_handler(handler_, ec));
                    } else {
                        if (time_out_ != 0) {
                            timer_.expires_from_now(boost::posix_time::milliseconds(time_out_));
                            timer_.async_wait(boost::bind(*this,
                                boost::asio::placeholders::error, true));

                        socket_.async_connect(e, boost::bind(*this,
                            boost::asio::placeholders::error, false));
                        } else {
                            socket_.async_connect(e, *this);
                        }
                    }
                }

                void start(
                    netname_type const & netname)
                {
                    boost::system::error_code ec;
                    boost::asio::detail::mutex::scoped_lock lock(mutex_);
                    if (canceled_) {
                        ec = boost::asio::error::operation_aborted;
                    } else {
                        // 在加锁的情况下启动
                        resolver_.async_resolve(netname, *this);
                        return;
                    }
                    canceled_ = false;
                    socket_.get_io_service().post(
                        boost::bind(handler_, ec));
                }

            public:
                void operator ()( // handle connect and timer
                    boost::system::error_code const & ec, bool is_timer)
                {
                    if (async_ret_.get_count() == 0) { // first time
                        async_ret_.reset();
                        if (is_timer) { // timer callback
                            async_ret_.set_ec(boost::asio::error::timed_out);
                            socket_.close();
                        } else { // connect callback
                            async_ret_.set_ec(ec);
                            timer_.cancel();
                        }
                        async_ret_.set_count(1);
                    } else { // second time
                        if (is_timer) { // timer callback
                            // nothing todo
                        } else { // connect callback
                            // nothing todo
                        }
                        async_ret_.set_count(0);
                        operator ()(async_ret_.get_ec());
                    }
                }

                void operator ()( // handle_resolve
                    boost::system::error_code const & ec, 
                    iterator iter)
                {
                    stat_.resolve_time = stat_.elapse();

                    boost::system::error_code ec1 = ec;

                    iterator_ = iter;
                    iterator end;
                    if (!ec1 && iterator_ != end) {
                        endpoint_type e = *iterator_;
                        {
                            boost::asio::detail::mutex::scoped_lock lock(mutex_);
                            if (canceled_) {
                                ec1 = boost::asio::error::operation_aborted;
                            } else {
                                boost::asio::socket_base::non_blocking_io cmd(non_block_);
                                socket_.close(ec1);
                                socket_.open(base_endpoint_type(e).protocol(), ec1) 
                                    || socket_.io_control(cmd, ec1);
                            }
                        }
                        if (!ec1) {
                            last_ip_ = e.ip_str();
                            LOG_S(framework::logger::Logger::kLevelDebug, "[async_connect] try server, ep: " << e.to_string());
                            if (time_out_ != 0) {
                                timer_.expires_from_now(boost::posix_time::milliseconds(time_out_));
                                timer_.async_wait(boost::bind(*this,
                                    boost::asio::placeholders::error, true));

                                socket_.async_connect(e, boost::bind(*this,
                                    boost::asio::placeholders::error, false));
                            } else {
                                socket_.async_connect(e, *this);
                            }
                            return;
                        }
                        stat_.connect_time = stat_.elapse();
                    }
                    LOG_S(framework::logger::Logger::kLevelDebug, "[async_connect] finish, ep: " 
                        << iterator_->to_string() << ", ec: " << ec1.message());
                    canceled_ = false;
                    handler_(ec1);
                }

                void operator ()( // handle connect
                    boost::system::error_code const & ec)
                {
                    LOG_SECTION();

                    boost::system::error_code ec1 = ec;
                    if (ec1) {
                        LOG_S(framework::logger::Logger::kLevelDebug, "[async_connect] failed, ep: " 
                            << iterator_->to_string() << ", ec: " << ec1.message());
                        iterator end;
                        /* 通过ip连接，iterator_一开始就等于end */
                        if (iterator_ != end && ++iterator_ != end) {
                            ec1.clear();
                            endpoint_type e = *iterator_;
                            {
                                boost::asio::detail::mutex::scoped_lock lock(mutex_);
                                if (canceled_) {
                                    ec1 = boost::asio::error::operation_aborted;
                                } else {
                                    boost::asio::socket_base::non_blocking_io cmd(non_block_);
                                    socket_.close(ec1);
                                    socket_.open(base_endpoint_type(e).protocol(), ec1) 
                                        || socket_.io_control(cmd, ec1);
                                }
                            }
                            if (!ec1) {
                                last_ip_ = e.ip_str();
                                LOG_SECTION();
                                LOG_S(framework::logger::Logger::kLevelDebug, "[async_connect] try server, ep: " << e.to_string());
                                if (time_out_ != 0) {
                                    timer_.expires_from_now(boost::posix_time::milliseconds(time_out_));
                                    timer_.async_wait(boost::bind(*this,
                                        boost::asio::placeholders::error, true));

                                    socket_.async_connect(e, boost::bind(*this,
                                        boost::asio::placeholders::error, false));
                                } else {
                                    socket_.async_connect(e, *this);
                                }
                                return;
                            }
                        }
                    }
                    LOG_S(framework::logger::Logger::kLevelDebug, "[async_connect] finish, ep: " 
                        << iterator_->to_string() << ", ec: " << ec1.message());
                    stat_.connect_time = stat_.elapse();
                    canceled_ = false;
                    handler_(ec1);
                }

                PASS_DOWN_ASIO_HANDLER_FUNCTION(connect_handler, handler_)

            private:
                socket & socket_; // TCP套接字
                bool non_block_;
                boost::asio::detail::mutex & mutex_;
                bool & canceled_;
                resolver & resolver_; // DNS解析器
                iterator iterator_;
                detail::ConnectorStatistics & stat_;
                ConnectHandler handler_;
                boost::uint32_t time_out_;
                boost::asio::deadline_timer & timer_;
                async_ret_type & async_ret_;
                std::string& last_ip_;
            };

        } // namespace detail

        template <typename Protocol, typename SocketType>
        template <typename ConnectHandler>
        void Connector<Protocol, SocketType>::async_connect(
            socket_type & peer, // 外部创建的套接字，不需要open
            netname_type const & netname, 
            ConnectHandler const & handler)
        {
            canceled_ = canceled_forever_;
            use_ip_ = netname.host();
            if (netname.is_digit()) {
                stat_.reset();
                stat_.resolve_time = 0;
                base_endpoint_type e;
                boost::system::error_code ec;
                e.address(boost::asio::ip::address::from_string(netname.host(), ec));
                if (ec) {
                    stat_.connect_time = stat_.elapse();
                    peer.get_io_service().post(boost::asio::detail::bind_handler(handler, ec));
                }
                e.port(netname.port());
                detail::connect_handler<Protocol, ConnectHandler> connect_handler(
                    peer, non_block_, mutex_, resolver_, canceled_, stat_, handler, time_out_, timer_, async_ret_, last_ip_);
                connect_handler.start(e);
            } else {
                stat_.reset();
                detail::connect_handler<Protocol, ConnectHandler> connect_handler(
                    peer, non_block_, mutex_, resolver_, canceled_, stat_, handler, time_out_, timer_, async_ret_, last_ip_);
                connect_handler.start(netname);
            }
        }

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_CONNECTOR_H_
