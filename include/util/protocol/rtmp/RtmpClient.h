// RtmpClient.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_CLIENT_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_CLIENT_H_

#include "util/protocol/rtmp/RtmpSocket.h"
#include "util/protocol/rtmp/RtmpMessage.h"

#include <framework/string/Url.h>

#include <boost/function.hpp>

namespace util
{
    namespace protocol
    {

        class RtmpClient
            : public RtmpSocket
        {
        public:
            typedef boost::function<
                void ( 
                boost::system::error_code const &)> response_type;

        public:
            RtmpClient(
                boost::asio::io_service & io_svc);

            ~RtmpClient();

        public:
            boost::system::error_code connect(
                framework::string::Url const & url, 
                boost::system::error_code & ec);

            void async_connect(
                framework::string::Url const & url, 
                response_type const & resp);

            boost::system::error_code play(
                boost::system::error_code & ec);

            void async_play(
                response_type const & resp);

            boost::system::error_code publish(
                boost::system::error_code & ec);

            void async_publish(
                response_type const & resp);

            void tick(
                boost::system::error_code & ec);

            void close();

            boost::system::error_code close(
                boost::system::error_code & ec);

        protected:
            void async_reqeust();

        private:
            enum ConnectionStatusEnum
            {
                closed, 
                connectting, 
                net_connectting, 
                established, 
                broken, 
            };

            enum RequestStatusEnum
            {
                finished, 
                send_pending, 
                sending_req, 
                recving_resp, 
            };

        private:
            void make_connect_requests(
                framework::string::Url const & url);
            
            void make_play_requests(
                std::string const & content);

            void make_publish_requests(
                std::string const & content);

            void resume_connect(
                boost::system::error_code & ec);

            void resume_request(
                boost::system::error_code & ec);

            void handle_async_connect(
                boost::system::error_code const & ec);

            void handle_async_reqeust(
                boost::system::error_code ec);

            bool post_response(
                boost::system::error_code & ec);

            void response(
                boost::system::error_code const & ec);

            void dump(
                char const * function, 
                boost::system::error_code const & ec);

            void dump_request(
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
            framework::string::Url url_;
            std::string content_;
            framework::network::NetName addr_;
            ConnectionStatusEnum status_;
            RequestStatusEnum request_status_;
            std::STREAMSDK_CONTANINER<RtmpMessage> requests_;
            RtmpMessage response_;
            std::vector<RtmpMessage> proto_responses_;
            response_type resp_;

        private:
            size_t id_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_CLIENT_H_
