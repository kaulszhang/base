// DataViewProxy.cpp

#include "util/Util.h"
#include "util/dataview/DataViewProxy.h"
#include "util/dataview/DataViewError.h"
#include "util/dataview/CatalogNode.h"
#include "util/dataview/DataView.h"
#include "util/dataview/PacketDataView.h"
using namespace util::protocol;
using namespace util::dataview::error;

#include <framework/string/Url.h>
using namespace framework::network;

#include <boost/lexical_cast.hpp>
using namespace boost::system;

#include <list>

namespace util
{
    namespace dataview
    {
        struct Session
        {
            static size_t currentid;
            typedef size_t IdType;
            size_t id;
            std::list<Path> lines;

            Session()
            {
                id = currentid++;
            }

            friend bool operator < (
                Session const & l, 
                Session const & r)
            {
                return l.id < r.id;
            }

            error_code add_line(
                Path const & path)
            {
                Data data;
                error_code ec = util::dataview::root().get_child_data(path, data);
                if (ec == error_succeed) {
                    if (data.type == DataType::t_text)
                        ec = error_data_type;
                }
                if (ec == error_succeed) {
                    //ec = lines.push_back(path);
                    // TODO, 异常，如果插入的元素以及存在，设置ec
                    lines.push_back(path);
                }
                return ec;
            }

            error_code del_line(
                Path const & path)
            {
                error_code ec;
                // 如果ec 已经元素不存在， 设置ec
                lines.remove(path);
                return ec;
            }

            error_code get_line_data(
                std::vector<Data> & line_datas) const
            {
                for (std::list<Path>::const_iterator i = lines.begin(); i != lines.end(); ++i) {
                    Data data;
                    error_code ec = util::dataview::root().get_child_data(*i, data);
                    if (ec != error_succeed) {
                        data.type = DataType::t_nodata;
                        data.v_text = ec.message();
                    }
                    line_datas.push_back(data);
                }
                return error_succeed;
            }
        };

        size_t Session::currentid = 0;

        std::list<Session> sessions;

        DataViewProxy::DataViewProxy(
            boost::asio::io_service & io_svc)
            : HttpProxy(io_svc)
            , contentLength_(0)
        {
        }

        bool DataViewProxy::on_receive_request_head(
            HttpRequestHead & request_head)
        {
            // Server模式：返回false
            // Proxy模式：返回true，可以修改request_head
            boost::system::error_code ec;
            request_head.get_content(std::cout, ec);
            parse_request(request_head.path);
            contentLength_ = response_buffer_.size();
            return false;
        }

        void DataViewProxy::on_receive_response_head( 
            HttpResponseHead & response_head)
        {
            // Server模式：设置response_head
            // Proxy模式：可以修改response_head
            response_head.version = 0x101;
            response_head.err_msg = "OK";
            response_head.set_field("Content-Length", boost::lexical_cast<std::string>(contentLength_));
            response_head.connection = util::protocol::http_field::Connection::keep_alive;

            boost::system::error_code ec;
            response_head.get_content(std::cout, ec);
        }

        void DataViewProxy::on_receive_response_data(
            boost::asio::streambuf & response_data)
        {
            std::ostream os(&response_data);
            os.write(response_buffer_.c_str(), contentLength_);
        }

        void DataViewProxy::parse_request(std::string const & url)
        {
            std::string tmphost = "http://host";
            tmphost = tmphost + url;
            framework::string::Url request_url(tmphost);

            std::ostringstream buffer;
            oarchive oa(buffer);
            if (request_url.path().compare("/login") == 0){
                sessions.push_back(Session());
                packet_login login;
                login.output.uid = sessions.back().id;
                oa << login.output;
            } else if (request_url.path().compare("/logout") == 0) {
                size_t uid = atoi(request_url.param("uid").c_str());
                for (std::list<Session>::const_iterator iter = sessions.begin(); iter != sessions.end(); iter++ )
                {
                    if ((*iter).id == uid) {
                        //iter = sessions.erase(iter);
                        break;
                    }
                }
            } /*else if (request_url.path().compare("/get_table_data") == 0) {
                util::dataview::Path path(request_url.param("path"));
                int from = atoi(request_url.param("from").c_str());
                int size = atoi(request_url.param("size").c_str());
                packet_get_table_data table_data;
                util::dataview::root().get_table_data(path, from, size, table_data.output.table_data);
                oa << table_data.output;
            } else if (request_url.path().compare("/get_children") == 0) {
                int uid = atoi(request_url.param("uid").c_str());
                util::dataview::Path path(request_url.param("path"));
                packet_get_children children;
                util::dataview::root().get_children(path, children.output.children);
                oa << children.output;
            } else if (request_url.path().compare("/get_table_head") == 0) {
                int uid = atoi(request_url.param("uid").c_str());
                util::dataview::Path path(request_url.param("path"));
                packet_get_table_head tablehead;
                util::dataview::root().get_table_head(path, tablehead.output.table_head);
                oa << tablehead.output;
            } else if (request_url.path().compare("/add_line") == 0) {
                int uid = atoi(request_url.param("uid").c_str());
                util::dataview::Path path(request_url.param("path"));
                add_line_to_session(path, uid);
            } 
            else if (request_url.path().compare("/del_line") == 0) {
                int uid = atoi(request_url.param("uid").c_str());
                util::dataview::Path path(request_url.param("path"));
                del_line_from_session(path, uid);
            } else if (request_url.path().compare("/get_line_data") == 0) {
                int uid = atoi(request_url.param("uid").c_str());
                packet_get_line_data line_datas;
                get_line_data(uid, line_datas.output.line_datas);
                oa << line_datas.output;
            }*/
            response_buffer_ = buffer.str();
        }

        void DataViewProxy::add_line_to_session(util::dataview::Path const & path, size_t uid)
        {
            Session session;
            for (std::list<Session>::const_iterator iter = sessions.begin(); iter != sessions.end(); iter++ )
            {
                if ((*iter).id == uid) {
                    session = *iter;
                    break;
                }
            }
            session.add_line(path);
        }

        void DataViewProxy::del_line_from_session(util::dataview::Path const &path, size_t uid)
        {
            Session session;
            for (std::list<Session>::const_iterator iter = sessions.begin(); iter != sessions.end(); iter++ )
            {
                if ((*iter).id == uid) {
                    session = *iter;
                    break;
                }
            }
            session.del_line(path);
        }

        void DataViewProxy::get_line_data(size_t uid, std::vector<util::dataview::Data> & line_datas)
        {
            for (std::list<Session>::const_iterator iter = sessions.begin(); iter != sessions.end(); iter++ )
            {
                if ((*iter).id == uid) {
                    (*iter).get_line_data(line_datas);
                    break;
                }
            }
        }

        void DataViewProxy::on_error(
            boost::system::error_code const & ec)
        {
            std::cout << ec.message() << std::endl;
        }

        DataViewServer::DataViewServer(boost::asio::io_service & io_srv
                                       , NetName addr)
                                       :io_srv_(io_srv)
                                       , addr_(addr)
                                       , mgr_(new HttpProxyManager<DataViewProxy>(io_srv_, addr_))
        {
        }

        DataViewServer::~DataViewServer()
        {
            if (mgr_) {
                delete mgr_;
                mgr_ = NULL;
            }
        }

        error_code DataViewServer::start()
        {
            error_code ec;
            mgr_->start();
            return ec;
        }

        error_code DataViewServer::stop()
        {
            error_code ec;
            return ec;
        }

    } // namespaace dataview
} // namespace util

