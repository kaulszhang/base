// HttpProxy.h

#ifndef _UTIL_PROTOCOL_HTTP_PROXY_H_
#define _UTIL_PROTOCOL_HTTP_PROXY_H_

#include "util/protocol/http/HttpSocket.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"

#include <framework/network/NetName.h>

#include <boost/function.hpp>

namespace util
{
    namespace protocol
    {

        class HttpProxy
        {
        public:
            HttpProxy(
                boost::asio::io_service & io_svc);

            virtual ~HttpProxy();

        public:
            void close();

            void cancel();

            boost::system::error_code cancel(
                boost::system::error_code & ec);

        private:
        protected:
            class Size
            {
            public:
                Size() : type_(0) {}
                Size(bool b) : type_(1), s1_(b ? 1 : 0) {}
                Size(int s): type_(2), s1_(s) {}
                Size(size_t s): type_(2), s1_(s) {}
                Size(size_t s1, size_t s2): type_(3), s1_(s1), s2_(s2) {}
                Size(std::pair<const size_t, const size_t> const & s): type_(3), s1_(s.first), s2_(s.second) {}
            public:
                bool is_bool() const { return type_ == 1; }
                bool is_size_t() const { return type_ == 2; }
                bool is_size_pair() const { return type_ == 3; }
            public:
                bool get_bool() const { assert(type_ == 1); return s1_ == 1; }
                size_t get_size_t() const { assert(type_ == 2); return s1_; }
                std::pair<size_t, size_t> get_size_pair() const 
                { assert(type_ == 3); return std::pair<size_t, size_t>(s1_, s2_); }
                std::string to_string() const;
            private:
                int type_;
                size_t s1_;
                size_t s2_;
            };


            typedef boost::function<void (
                boost::system::error_code const &, 
                Size const &)> response_type;

        protected:
            HttpRequest & get_request()
            {
                return request_;
            }

            HttpResponse & get_response()
            {
                return response_;
            }

            HttpRequestHead & get_request_head()
            {
                return request_.head();
            }

            HttpResponseHead & get_response_head()
            {
                return response_.head();
            }

            HttpSocket & get_client_data_stream()
            {
                return http_to_client_;
            }

            HttpSocket & get_server_data_stream()
            {
                return *http_to_server_;
            }

            bool is_local()
            {
                return http_to_server_ == NULL;
            }

        protected:
            HttpRequest & request()
            {
                return request_;
            }

            HttpResponse & response()
            {
                return response_;
            }

            HttpRequestHead & request_head()
            {
                return request_.head();
            }

            HttpResponseHead & response_head()
            {
                return response_.head();
            }

            boost::asio::streambuf & request_data()
            {
                return request_.data();
            }

            boost::asio::streambuf & response_data()
            {
                return response_.data();
            }

            HttpSocket & client_data_stream()
            {
                return http_to_client_;
            }

            HttpSocket & server_data_stream()
            {
                return *http_to_server_;
            }

        protected:
            virtual void on_receive_request_head(
                HttpRequestHead & request_head, 
                response_type const & resp)
            {
                // Server模式：返回false
                // Proxy模式：返回true，可以修改request_head
                resp(boost::system::error_code(), 
                    on_receive_request_head(request_head));
            }

            virtual bool on_receive_request_head(
                HttpRequestHead & request_head)
            {
                // Server模式：返回false
                // Proxy模式：返回true，可以修改request_head
                return true;
            }

            virtual void on_receive_request_data(
                boost::asio::streambuf & request_data)
            {
                // Server模式
            }

            virtual void local_process(
                response_type const & resp);

            virtual void on_receive_response_head(
                HttpResponseHead & response_head)
            {
                // Server模式：设置response_head
                // Proxy模式：可以修改response_head
            }

            virtual void on_receive_response_data(
                boost::asio::streambuf & response_data)
            {
                // Server模式
            }

            virtual void on_error(
                boost::system::error_code const & ec)
            {
            }

            virtual void on_finish()
            {
            }

            // 提醒派生类放弃
            virtual void on_broken_pipe();

            virtual void transfer_request_data(
                response_type const & resp);

            virtual void transfer_response_data(
                response_type const & resp);

        protected:
            enum StateEnum
            {
                stopped, 
                receiving_request_head, 
                preparing, 
                connectting, 
                sending_request_head, 
                transferring_request_data, 
                local_processing, 
                receiving_response_head, 
                sending_response_head, 
                transferring_response_data, 
                exiting, 
            };

            StateEnum state() const
            {
                return state_;
            };

        private:
            static std::string const state_str[];

        private:
            void start();

            void handle_watch(
                boost::system::error_code const & ec);

            void handle_async(
                boost::system::error_code const & ec, 
                Size const & bytes_transferred);

            void response_error(
                boost::system::error_code const & ec, 
                response_type const & resp);

        private:
            template <
                typename HttpProxy, 
                typename Manager
            >
            friend class HttpProxyManager;

            enum WatchStateEnum
            {
                watch_stopped, 
                watching, 
                broken, 
            };

            size_t id_;
            StateEnum state_;
            WatchStateEnum watch_state_;
            HttpSocket http_to_client_;
            HttpSocket * http_to_server_;
            HttpRequest request_;
            HttpResponse response_;
            boost::asio::streambuf transfer_buf_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_PROXY_H_
