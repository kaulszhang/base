// HttpProxy.cpp

#include "util/Util.h"
#include "util/protocol/http/HttpProxy.h"
#include "util/stream/StreamTransfer.h"
using namespace util::stream;

#include <boost/bind.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/completion_condition.hpp>
using namespace boost::system;

namespace util
{
    namespace protocol
    {

        static size_t const DATA_BUFFER_SIZE = 10240;

        void HttpProxy::transfer_request_data(
            response_type const & resp)
        {
            transfer_buf_.reset();
            size_t content_length = request_.head().content_length.get_value_or(0);
            if (is_local()) {
                if (content_length) {
                    boost::asio::async_read(
                        http_to_client_, 
                        transfer_buf_, 
                        boost::asio::transfer_at_least(content_length), 
                        resp);
                } else {
                    resp(boost::system::error_code(), Size());
                }
            } else {
                if (content_length) {
                    async_transfer(
                        http_to_client_, 
                        *http_to_server_, 
                        transfer_buf_.prepare(DATA_BUFFER_SIZE), 
                        transfer_at_least(content_length), 
                        resp);
                } else {
                    resp(boost::system::error_code(), Size());
                }
            }
        }

        void HttpProxy::transfer_response_data(
            response_type const & resp)
        {
            transfer_buf_.reset();
            if (is_local()) {
                on_receive_response_data(transfer_buf_);
                if (transfer_buf_.size()) {
                    boost::asio::async_write(http_to_client_, transfer_buf_, resp);
                } else {
                    resp(boost::system::error_code(), Size());
                }
            } else {
                if (response_.head().content_length.is_initialized()) {
                    size_t content_length = response_.head().content_length.get();
                    if (content_length) {
                        async_transfer(
                            *http_to_server_, 
                            http_to_client_, 
                            transfer_buf_.prepare(DATA_BUFFER_SIZE), 
                            transfer_at_least(content_length), 
                            resp);
                    } else {
                        resp(boost::system::error_code(), Size());
                    }
                } else {
                    async_transfer(
                        *http_to_server_, 
                        http_to_client_, 
                        transfer_buf_.prepare(DATA_BUFFER_SIZE), 
                        resp);
                }
            }
        }

    } // namespace protocol
} // namespace util
