// RtspServer.h

#ifndef _UTIL_PROTOCOL_RTSP_SERVER_H_
#define _UTIL_PROTOCOL_RTSP_SERVER_H_

#include "util/protocol/rtsp/RtspSocket.h"
#include "util/protocol/rtsp/RtspRequest.h"
#include "util/protocol/rtsp/RtspResponse.h"
#include "util/stream/StreamTransfer.h"

#include <framework/network/NetName.h>

#include <boost/function/function1.hpp>
#include <boost/function/function2.hpp>

namespace util
{
    namespace protocol
    {

        class RtspServer
            : protected RtspSocket
        {
        public:
            RtspServer(
                boost::asio::io_service & io_svc);

            virtual ~RtspServer();

        protected:
            typedef boost::function1<void, 
                boost::system::error_code const &
            > local_process_response_type;

        protected:
            RtspRequest & request()
            {
                return request_;
            }

            RtspResponse & response()
            {
                return response_;
            }

        protected:
            virtual void local_process(
                local_process_response_type const & resp)
            {
                resp(boost::system::error_code());
            }

            virtual void on_error(
                boost::system::error_code const & ec)
            {
            }

            virtual void on_finish()
            {
            }

        private:
            void start();

        private:
            void handle_prepare(
                boost::system::error_code const & ec, 
                bool proxy);

            void handle_receive_request_head(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_connect_server(
                boost::system::error_code const & ec);

            void handle_send_request_head(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_receive_request_data(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_transfer_request_data(
                boost::system::error_code const & ec, 
                util::stream::transfer_size const & bytes_transferred);

            void handle_local_process(
                boost::system::error_code const & ec);

            void handle_receive_response_head(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_send_response_head(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_send_response_data(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_transfer_response_data(
                boost::system::error_code const & ec, 
                util::stream::transfer_size const & bytes_transferred);

            void handle_response_error(
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

            void handle_error(
                boost::system::error_code const & ec);

            void response_error(
                boost::system::error_code const & ec);

        private:
            template <typename RtspServer, typename Manager>
            friend class RtspServerManager;

            size_t id_;
            RtspRequest request_;
            RtspResponse response_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_SERVER_H_
