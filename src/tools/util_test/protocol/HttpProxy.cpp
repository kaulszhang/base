// HttpProxy.cpp

#include "tools/util_test/Common.h"

#include "util/protocol/http/HttpProxyManager.h"
#include "util/protocol/http/HttpProxy.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"
#include "util/stream/StreamTransfer.h"
using namespace util::protocol;
using namespace util::stream;
using namespace framework::configure;
using namespace framework::logger;

#include <framework/logger/LoggerFormatRecord.h>
#include <framework/network/NetName.h>
using namespace framework::network;

#include <boost/asio/read.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
using namespace boost::system;
using namespace boost::asio;
using namespace boost::asio::ip;

#include <iostream>

class MyHttpProxy
    : public HttpProxy
{
public:
    MyHttpProxy(
        boost::asio::io_service & io_svc)
        : HttpProxy(io_svc)
    {
    }

    virtual bool on_receive_request_head(
        HttpRequestHead & request_head)
    {
        // Server模式：返回false
        // Proxy模式：返回true，可以修改request_head
        
        request_head.get_content(std::cout);

        return true;
    }

    virtual void on_receive_response_head( 
        HttpResponseHead & response_head)
    {
        // Server模式：设置response_head
        // Proxy模式：可以修改response_head

        response_head.get_content(std::cout);
    }

    virtual void transfer_request_data(
        response_type const & resp)
    {
        if (is_local()) {
            HttpProxy::transfer_request_data(resp); // 将会调用on_receive_request_data
        } else {
            HttpProxy::transfer_request_data(resp);
        }
    }

    virtual void transfer_response_data(
        response_type const & resp)
    {
        if (is_local()) {
            HttpProxy::transfer_response_data(resp); // 将会调用on_receive_response_data
        } else {
            HttpProxy::transfer_response_data(resp); // 将会调用on_receive_response_data
            //resp_ = resp;
            //size_t len = 0;
            //boost::asio::async_read(get_server_data_stream(), response_buffer_.prepare(len), 
            //    bind(boost::mem_fn(&MyHttpProxy::handle_receive_response_data), this, _1, _2));
        }
    }

    virtual void on_receive_request_data(
        boost::asio::streambuf & request_data)
    {
        // 读出请求内容
        std::istream is(&request_data);
        int i = 0;
        std::string str;
        is >> i >> str;
    }

    virtual void on_receive_response_data(
        boost::asio::streambuf & response_data)
    {
        // 写入应答内容
        std::ostream os(&response_data);
        int i = 0;
        std::string str = "str";
        os << i << str;
    }

    virtual void on_error(
        boost::system::error_code const & ec)
    {
        std::cout << ec.message() << std::endl;
    }

    //void handle_receive_response_data(
    //    boost::system::error_code const & ec, 
    //    size_t bytes_transferred)
    //{
    //    response_type resp;
    //    resp.swap(resp_);
    //    async_transfer(get_server_data_stream(), get_client_data_stream(), response_buffer_.prepare(4096), resp);
    //}

    boost::asio::streambuf response_buffer_;
    response_type resp_;
};

FRAMEWORK_LOGGER_DECLARE_MODULE("TestHttpProxy");

void test_http_proxy(Config & conf)
{
    NetName addr("0.0.0.0", 8080);

    conf.register_module("TestHttpProxy")
        << CONFIG_PARAM_RDWR(addr);

    {
        LoggerSection ls(glog);
        LOG_F(Logger::kLevelDebug, ("[test_http_proxy] start"));
        io_service io_svc;
        HttpProxyManager<MyHttpProxy> mgr(io_svc, addr);
        mgr.start();
        io_svc.run();
        LOG_F(Logger::kLevelDebug, ("[test_http_proxy] end"));
    }
}

static TestRegister tr("http_proxy", test_http_proxy);
