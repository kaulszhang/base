// MmsServer.h

#ifndef _UTIL_PROTOCOL_MMS_SERVER_H_
#define _UTIL_PROTOCOL_MMS_SERVER_H_

#include "util/protocol/mms/MmsSocket.h"
#include "util/protocol/mms/MmsRequest.h"
#include "util/protocol/mms/MmsResponse.h"
#include "util/stream/StreamTransfer.h"

#include <framework/network/NetName.h>

#include <boost/function/function1.hpp>
#include <boost/function/function2.hpp>

namespace util
{
    namespace protocol
    {

        class MmsServer
            : protected MmsSocket
        {
        public:
            MmsServer(
                boost::asio::io_service & io_svc);

            virtual ~MmsServer();

        protected:
            typedef boost::function1<void, 
                boost::system::error_code const &
            > local_process_response_type;

        protected:
            MmsRequest & request()
            {
                return request_;
            }

            MmsResponse & response()
            {
                return response_;
            }

        protected:
            virtual void local_process(
                local_process_response_type const & resp)
            {
                resp(boost::system::error_code());
            }

            virtual void process_response(
                boost::system::error_code const & ec,
                boost::asio::streambuf const & resp_buf);

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
            template <typename MmsServer, typename Manager>
            friend class MmsServerManager;

            size_t id_;
            MmsRequest request_;
            MmsResponse response_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMS_SERVER_H_
