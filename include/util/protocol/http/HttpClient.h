// HttpClient.h

#ifndef _UTIL_PROTOCOL_HTTP_CLIENT_H_
#define _UTIL_PROTOCOL_HTTP_CLIENT_H_

#include "util/protocol/http/HttpSocket.h"
#include "util/protocol/http/HttpsSocket.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"
#include "util/protocol/http/HttpError.h"

#include <framework/string/Url.h>

#include <boost/function/function1.hpp>

namespace util
{
    namespace protocol
    {
        struct HostParams
        {
            HostParams(std::string const & key, std::string const &value)
                : key_(key)
                , value_(value)
            {

            }

            HostParams(std::string const & str)
            {
                std::string::size_type pos_eq =
                    str.find('=');
                if (pos_eq < str.size()) {

                    key_ = str.substr(0, pos_eq);
                    value_ = str.substr(pos_eq + 1);
                }
            }

            std::string to_string()
            {
                return (key_ + "=" + value_);
            }

            bool is_inited()
            {
                return (!key_.empty() && !value_.empty());
            }

            bool operator==(const HostParams& obj)
            {
                return (key_ == obj.key_);
            }

            std::string key_;
            std::string value_;

        };

        class HttpSocketEx
        {
        public:
            HttpSocketEx(boost::asio::io_service & io_svc)
                : io_svc_(io_svc)
            {
                http_socket_ = new HttpSocket(io_svc);
                https_socket_ = NULL;
            }

            virtual ~HttpSocketEx()
            {
                clear_http_socket();
            }

            boost::asio::io_service & get_io_service()
            {
                return io_svc_;
            }

            bool get_non_block(
                boost::system::error_code & ec) const
            {
                if (http_socket_)
                    return http_socket_->get_non_block(ec);
                else
                    return https_socket_->get_non_block(ec);
            }

            boost::system::error_code close(
                boost::system::error_code & ec)
            {
                if (http_socket_)
                    return http_socket_->close(ec);
                else
                    return https_socket_->close(ec);
            }

            void get_resolvered_host_list(std::string& hosts)
            {
                hosts.clear();
                get_host_list();

                std::list<HostParams>::iterator iter = ip_list_.begin();
                for (; iter != ip_list_.end(); ++iter)
                {
                    if (iter != ip_list_.begin())
                        hosts += "|";
                    hosts += iter->to_string();
                }
            }

            void close()
            {
                if (http_socket_)
                    return http_socket_->close();
                else
                    return https_socket_->close();
            }

            boost::system::error_code cancel_forever(
                boost::system::error_code & ec)
            {
                if (http_socket_)
                    return http_socket_->cancel_forever(ec);
                else
                    return https_socket_->cancel_forever(ec);
            }

            boost::system::error_code cancel(
                boost::system::error_code & ec)
            {
                if (http_socket_)
                    return http_socket_->cancel(ec);
                else
                    return https_socket_->cancel(ec);
            }

            void cancel()
            {
                if (http_socket_)
                    http_socket_->cancel();
                else
                    https_socket_->cancel();
            }

            boost::system::error_code set_time_out(
                boost::uint32_t time_out,
                boost::system::error_code & ec)
            {
                if (http_socket_)
                    return http_socket_->set_time_out(time_out,ec);
                else
                    return https_socket_->set_time_out(time_out, ec);
            }


            template <typename Handler>
            void async_write(
                HttpHead & head, 
                Handler const & handler)
            {
                if (http_socket_)
                    http_socket_->async_write(head, handler);
                else
                    https_socket_->async_write(head, handler);
            }
            
            template <typename Handler>
            void async_read(
                HttpHead & head, 
                Handler const & handler)
            {
                if (http_socket_)
                    http_socket_->async_read(head, handler);
                else
                    https_socket_->async_read(head, handler);
            }

            template <typename Handler>
            void async_connect(
                framework::network::NetName const & addr,
                Handler const & resp)
            {
                framework::network::NetName addrTmp = addr;
                create_socket(addrTmp);
                if (http_socket_)
                    http_socket_->async_connect(addrTmp, resp);
                else
                    https_socket_->async_connect(addrTmp, resp);
            }

            void connect(
                framework::network::NetName const & addr
                )
            {
                framework::network::NetName addrTmp = addr;
                create_socket(addrTmp);

                if (http_socket_)
                    http_socket_->connect(addrTmp);
                else
                    https_socket_->connect(addrTmp);
            }

            boost::system::error_code connect(
                framework::network::NetName const & addr,
                boost::system::error_code & ec)
            {
                framework::network::NetName addrTmp = addr;
                create_socket(addrTmp);

                if (http_socket_)
                    return http_socket_->connect(addrTmp, ec);
                else
                    return https_socket_->connect(addrTmp, ec);
            }

            size_t write(
                HttpHead & head)
            {
                if (http_socket_)
                    return http_socket_->write(head);
                else
                    return https_socket_->write(head);

            }

            size_t write(
                HttpHead & head,
                boost::system::error_code & ec)
            {
                if (http_socket_)
                    return http_socket_->write(head, ec);
                else
                    return https_socket_->write(head, ec);
            }

            template <typename ConstBufferSequence>
            std::size_t write_some(const ConstBufferSequence& buffers)
            {
                if (http_socket_)
                    return http_socket_->write_some(buffers);
                else
                    return https_socket_->write_some(buffers);
            }

            template <typename ConstBufferSequence>
            std::size_t write_some(const ConstBufferSequence& buffers, boost::system::error_code& ec)
            {
                if (http_socket_)
                    return http_socket_->write_some(buffers, ec);
                else
                    return https_socket_->write_some(buffers, ec);
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_write_some(const MutableBufferSequence& buffers, BOOST_ASIO_MOVE_ARG(ReadHandler) handler)
            {
                if (http_socket_)
                    http_socket_->async_write_some(buffers, handler);
                else
                    https_socket_->async_write_some(buffers, handler);
            }

            size_t read(
                HttpHead & head,
                boost::system::error_code & ec)
            {
                if (http_socket_)
                    return http_socket_->read(head, ec);
                else
                    return https_socket_->read(head, ec);
            }

            boost::system::error_code set_non_block(
                bool non_block,
                boost::system::error_code & ec)
            {
                if (http_socket_)
                    return http_socket_->set_non_block(non_block, ec);
                else
                    return https_socket_->set_non_block(non_block, ec);
            }

            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers,
                boost::system::error_code& ec)
            {
                if (http_socket_)
                    return http_socket_->read_some(buffers, ec);
                else
                    return https_socket_->read_some(buffers, ec);
            }

            template <typename MutableBufferSequence>
            std::size_t read_some(
                const MutableBufferSequence& buffers)
            {
                if (http_socket_)
                    return http_socket_->read_some(buffers);
                else
                    return https_socket_->read_some(buffers);
            }

            template <typename MutableBufferSequence, typename ReadHandler>
            void async_read_some(
                const MutableBufferSequence & buffers,
                ReadHandler handler)
            {
                if (http_socket_)
                    http_socket_->async_read_some(buffers, handler);
                else
                    https_socket_->async_read_some(buffers, handler);
            }

       private:

           void parse_netname_for_connect(framework::network::NetName & addr)
           {
               std::string host = addr.host();

               std::vector<std::string> args;
               bool find_com = false;
               framework::string::slice<std::string>(host, std::inserter(args, args.end()), ".");

               host.clear();

               for (size_t i = 0; i < args.size(); i++)
               {
                   if (args[i] == "com" && i > 0)
                   {
                       if (args[i - 1] != "sina")
                           break;

                       host = "www.";
                       host += args[i - 1];
                       find_com = true;
                   }

                   if (find_com)
                   {
                       host += ".";
                       host += args[i];
                   }
               }

               if (!host.empty())
                   addr.host(host);
           }

           void create_socket(framework::network::NetName & addr)
           {
               clear_http_socket();
               if (addr.app_protocol() == "https")
               {
                   if (addr.svc().empty())
                       addr.svc("443");
                   https_socket_ = new HttpsSocket(io_svc_);
               }
               else
               {
                   if (addr.svc().empty())
                       addr.svc("80");
                    http_socket_ = new HttpSocket(io_svc_);
               }
                   
           }

           void get_host_list()
           {
               std::string hosts;
               if (http_socket_)
               {
                   http_socket_->get_resolvered_host_list(hosts);
               }
               else if (https_socket_)
               {
                   https_socket_->get_resolvered_host_list(hosts);
               }

               if (hosts.empty())
                   return;

               HostParams param(hosts);
               if (!param.is_inited())
                   return;

               std::list<HostParams>::iterator iter = std::find(ip_list_.begin(), ip_list_.end(), param);
               if (iter != ip_list_.end())
                   ip_list_.erase(iter);
               ip_list_.push_back(param);
           }

           void clear_http_socket()
           {
               get_host_list();
               if (http_socket_)
                   delete http_socket_;
               if (https_socket_)
                   delete https_socket_;
               http_socket_ = NULL;
               https_socket_ = NULL;
           }

        private:
            boost::asio::io_service &io_svc_;
            HttpSocket *http_socket_;
            HttpsSocket *https_socket_;
            std::list<HostParams>  ip_list_;
        };



        class HttpClient : public HttpSocketEx
        {
        public:
            typedef boost::function1<
                void, 
                boost::system::error_code const &> response_type;

            struct Statistics
                : public HttpSocket::Statistics
            {
                void reset()
                {
                    HttpSocket::Statistics::reset();
                    send_pend_time 
                        = request_head_time 
                        = request_data_time 
                        = recv_pend_time 
                        = response_head_time 
                        = response_data_time
                        = (boost::uint32_t)-1;
                }

                boost::uint32_t send_pend_time;
                boost::uint32_t request_head_time;
                boost::uint32_t request_data_time;
                boost::uint32_t recv_pend_time;
                boost::uint32_t response_head_time;
                boost::uint32_t response_data_time;
            };

        public:
            static bool recoverable(
                boost::system::error_code const & ec)
            {
                return ec.category() == util::protocol::http_error::get_category() ? 
                    (ec.value() < util::protocol::http_error::ok || 
                    ec.value() >= util::protocol::http_error::internal_server_error) :
                (ec == boost::asio::error::network_reset || 
                    ec == boost::asio::error::timed_out || 
                    ec == boost::asio::error::network_down || 
                    ec == boost::asio::error::connection_reset || 
                    ec == boost::asio::error::connection_refused || 
                    ec == boost::asio::error::host_not_found || 
                    ec == boost::asio::error::eof || 
                    ec == boost::asio::error::host_unreachable);
            }

        public:
            static void set_http_proxy(std::string const & host);

            HttpClient(
                boost::asio::io_service & io_svc);

            ~HttpClient();

        public:
            boost::system::error_code bind_host(
                std::string const & host, 
                boost::system::error_code & ec);

            boost::system::error_code bind_host(
                std::string const & host, 
                std::string const & port, 
                boost::system::error_code & ec);

            boost::system::error_code bind_host(
                framework::network::NetName const & addr, 
                boost::system::error_code & ec);

        public:
            // 发送GET请求
            boost::system::error_code open(
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method, 
                boost::system::error_code & ec)
            {
                HttpRequestHead head;
                return open(url_to_head(head, url, method), ec);
            }

            boost::system::error_code open(
                framework::string::Url const & url, 
                boost::system::error_code & ec)
            {
                return open(url, HttpRequestHead::get, ec);
            }

            boost::system::error_code open(
                std::string const & url, 
                boost::system::error_code & ec)
            {
                return open(framework::string::Url(url), ec);
            }

            boost::system::error_code open(
                HttpRequestHead const & head, 
                boost::system::error_code & ec)
            {
                return open(HttpRequest(head), ec);
            }

            boost::system::error_code open(
                HttpRequest const & request, 
                boost::system::error_code & ec);

            boost::system::error_code reopen(
                boost::system::error_code & ec);

            bool is_open(
                boost::system::error_code & ec);

            void async_open(
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method, 
                response_type const & resp)
            {
                HttpRequestHead head;
                async_open(url_to_head(head, url, method), resp);
            }

            void async_open(
                framework::string::Url const & url, 
                response_type const & resp)
            {
                async_open(url, HttpRequestHead::get, resp);
            }

            void async_open(
                std::string const & url, 
                response_type const & resp)
            {
                async_open(framework::string::Url(url), resp);
            }

            void async_open(
                HttpRequestHead const & head, 
                response_type const & resp)
            {
                async_open(HttpRequest(head), resp);
            }

            void async_open(
                HttpRequest const & request, 
                response_type const & resp);

            void async_reopen(
                response_type const & resp);

            boost::system::error_code read_finish(
                boost::system::error_code & ec, 
                boost::uint64_t bytes_transferred);

        public:
            void close();

            boost::system::error_code close(
                boost::system::error_code & ec);

        public:
            boost::system::error_code fetch(
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method, 
                boost::system::error_code & ec)
            {
                HttpRequestHead head;
                return fetch(url_to_head(head, url, method), ec);
            }

            boost::system::error_code fetch(
                framework::string::Url const & url, 
                boost::system::error_code & ec)
            {
                return fetch(url, HttpRequestHead::get, ec);
            }

            boost::system::error_code fetch(
                std::string const & url, 
                boost::system::error_code & ec)
            {
                return fetch(framework::string::Url(url), ec);
            }

            boost::system::error_code fetch(
                HttpRequestHead const & head, 
                boost::system::error_code & ec)
            {
                return fetch(HttpRequest(head), ec);
            }

            boost::system::error_code fetch(
                HttpRequest const & request, 
                boost::system::error_code & ec);

            boost::system::error_code refetch(
                boost::system::error_code & ec);

            bool is_fetch(
                boost::system::error_code & ec);

            boost::system::error_code fetch_get(
                std::string const & url, 
                boost::system::error_code & ec)
            {
                return fetch(framework::string::Url(url), HttpRequestHead::get, ec);
            }

            boost::system::error_code fetch_get(
                framework::string::Url const & url, 
                boost::system::error_code & ec)
            {
                return fetch(url, HttpRequestHead::get, ec);
            }

            boost::system::error_code fetch_post(
                std::string const & url, 
                boost::system::error_code & ec)
            {
                return fetch(framework::string::Url(url), HttpRequestHead::post, ec);
            }

            boost::system::error_code fetch_post(
                framework::string::Url const & url, 
                boost::system::error_code & ec)
            {
                return fetch(url, HttpRequestHead::post, ec);
            }

            void async_fetch(
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method, 
                response_type const & resp)
            {
                HttpRequestHead head;
                return async_fetch(url_to_head(head, url, method), resp);
            }

            void async_fetch(
                framework::string::Url const & url, 
                response_type const & resp)
            {
                async_fetch(url, HttpRequestHead::get, resp);
            }

            void async_fetch(
                std::string const & url, 
                response_type const & resp)
            {
                async_fetch(framework::string::Url(url), resp);
            }

            void async_fetch(
                HttpRequestHead const & head, 
                response_type const & resp)
            {
                return async_fetch(HttpRequest(head), resp);
            }

            void async_fetch(
                HttpRequest const & request, 
                response_type const & resp);

            void async_refetch(
                response_type const & resp);

            void async_fetch_get(
                std::string const & url, 
                response_type const & resp)
            {
                async_fetch(framework::string::Url(url), HttpRequestHead::get, resp);
            }

            void async_fetch_get(
                framework::string::Url const & url, 
                response_type const & resp)
            {
                async_fetch(url, HttpRequestHead::get, resp);
            }

            void async_fetch_post(
                std::string const & url, 
                response_type const & resp)
            {
                async_fetch(framework::string::Url(url), HttpRequestHead::post, resp);
            }

            void async_fetch_post(
                framework::string::Url const & url, 
                response_type const & resp)
            {
                async_fetch(url, HttpRequestHead::post, resp);
            }

        private:
            static HttpRequestHead const & url_to_head(
                HttpRequestHead & head, 
                framework::string::Url const & url, 
                HttpRequestHead::MethodEnum method)
            {
                head.protocol = url.protocol();
                head.method = method;
                head.path = url.path_all();
                if (!url.host().empty())
                    head.host.reset(url.host_svc());
                return head;
            }

        private:
            enum ConnectionStatusEnum
            {
                closed, 
                connectting, 
                established, 
                ready, // 第一个请求已经打开，并且所有请求已经发出去
                broken, 
            };

            enum RequestStatusEnum
            {
                send_pending, 
                sending_req_head, 
                sending_req_data, 
                recv_pending, 
                recving_resp_head, 
                opened, 
                recving_resp_data, 
                finished, 
            };

        private:
            struct Request
                : HttpRequest
            {
                Request(
                    size_t id, 
                    HttpRequest const & request, 
                    bool is_fetch)
                    : HttpRequest(request)
                    , id(id)
                    , is_fetch(is_fetch)
                    , is_async(false)
                    , status(send_pending)
                {
                }

                Request(
                    size_t id, 
                    HttpRequest const & request, 
                    bool is_fetch, 
                    response_type const & resp)
                    : HttpRequest(request)
                    , id(id)
                    , is_fetch(is_fetch)
                    , is_async(true)
                    , status(send_pending)
                    , resp(resp)
                {
                }

                size_t id;
                bool is_fetch;
                bool is_async;
                RequestStatusEnum status;
                Statistics stat;
                response_type resp;
            };

        public:
            HttpRequest & request()
            {
                return requests_[0];
            }

            HttpResponse & response()
            {
                return response_;
            }

            HttpRequestHead & request_head()
            {
                return request().head();
            }

            HttpResponseHead & response_head()
            {
                return response().head();
            }

            boost::asio::streambuf & request_data()
            {
                return request().data();
            }

            boost::asio::streambuf & response_data()
            {
                return response().data();
            }

			Statistics const & stat() const
            {
                return requests_[0].stat;
            }

        private:
            boost::system::error_code post_reqeust(
                HttpRequest const & request, 
                bool is_fetch, 
                response_type const & resp, 
                boost::system::error_code & ec);

            boost::system::error_code post_reqeust(
                HttpRequest const & request, 
                bool is_fetch, 
                boost::system::error_code & ec)
            {
                return post_reqeust(request, is_fetch, response_type(), ec);
            }

            boost::system::error_code resume(
                bool pending, 
                boost::system::error_code & ec);

            boost::system::error_code resume_connect(
                boost::system::error_code & ec);

            boost::system::error_code resume_request(
                bool pending, 
                boost::system::error_code & ec);

            void async_resume();

            void handle_async_connect(
                boost::system::error_code const & ec);

            void handle_async_reqeust(
                bool pending, 
                boost::system::error_code const & ec);

            void response_request(
                Request & request, 
                boost::system::error_code const & ec);

            bool handle_redirect(
                Request & request, 
                boost::system::error_code & ec);

            bool handle_next(
                boost::system::error_code & ec);

            void post_handle_request(
                Request & request, 
                boost::system::error_code & ec);

            void dump(
                char const * function, 
                boost::system::error_code const & ec);

            void dump_request(
                Request const & request, 
                char const * function, 
                boost::system::error_code const & ec);

            void close_socket(
                boost::system::error_code & ec);

        private:
            static std::string const con_status_str[];
            static std::string const req_status_str[];

        private:
            static size_t next_id_;

        private:
            framework::network::NetName addr_;
            ConnectionStatusEnum status_;
            boost::system::error_code broken_error_;
            bool is_keep_alive_;
	        std::STREAMSDK_CONTANINER<Request> requests_;
            HttpResponse response_;
            size_t req_id_;
            size_t num_sent_;   // 已经发出去的请求数

            static framework::network::NetName g_proxy_addr_;

        private:
            size_t id_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_CLIENT_H_
