// DataViewProxy.h

#ifndef _UTIL_DATAVIEW_DATAVIEW_PROXY_H_
#define _UTIL_DATAVIEW_DATAVIEW_PROXY_H_

#include "util/protocol/http/HttpProxyManager.h"
#include "util/protocol/http/HttpProxy.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"
#include "util/stream/StreamTransfer.h"

#include "util/dataview/CatalogReg.h"

#include <boost/shared_array.hpp>
#include <framework/network/NetName.h>

namespace util
{
    namespace dataview
    {
        class DataViewProxy
            : public util::protocol::HttpProxy
        {
        public:
            DataViewProxy(
                boost::asio::io_service & io_svc);

            virtual bool on_receive_request_head(
                util::protocol::HttpRequestHead & request_head);

            virtual void on_receive_response_head( 
                util::protocol::HttpResponseHead & response_head);

            virtual void on_receive_response_data(
                boost::asio::streambuf & response_data);

            void add_line_to_session(
                util::dataview::Path const & path, 
                size_t uid);

            void del_line_from_session(
                util::dataview::Path const & path, 
                size_t uid);

            void get_line_data(
                size_t uid, 
                std::vector<util::dataview::Data> & line_datas);

            virtual void on_error(
                boost::system::error_code const & ec);

           void parse_request(
               std::string const & url);

            //void local_process(local_process_response_type const & resp);

        private:
            std::string response_buffer_;
            response_type resp_;
            size_t contentLength_;
        };

        class DataViewServer
        {
        public:
            DataViewServer(
                 boost::asio::io_service & io_srv,
                 framework::network::NetName addr);

            ~DataViewServer();

            boost::system::error_code start();
            boost::system::error_code stop();

        private:
            boost::asio::io_service & io_srv_;
            framework::network::NetName addr_;
            util::protocol::HttpProxyManager<DataViewProxy> * mgr_;
        };

    } // namespace dataview
} // namespace util

#endif // _UTIL_DATAVIEW_DATAVIEW_PROXY_H_

