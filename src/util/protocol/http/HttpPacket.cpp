// HttpPacket.cpp

#include "util/Util.h"
#include "util/protocol/http/HttpPacket.h"
#include "util/protocol/http/HttpError.h"
#include "util/protocol/http/HttpRequest.h"
#include "util/protocol/http/HttpResponse.h"

#include <framework/string/Format.h>
#include <framework/string/Parse.h>
using namespace framework::string;
using namespace framework::system::logic_error;

using namespace boost::system;

#include <sstream>

#include <string.h>

namespace framework
{
    namespace string
    {
        std::string to_string(
            util::protocol::HttpRequestHead::MethodEnum const & e)
        {
            return util::protocol::HttpRequestHead::method_str[e];
        }

        error_code from_string(
            util::protocol::HttpRequestHead::MethodEnum & e, 
            std::string const & str)
        {
            for (int i = 0; i < util::protocol::HttpRequestHead::invalid_method; ++i) {
                if (util::protocol::HttpRequestHead::method_str[i] == str) {
                    e = (util::protocol::HttpRequestHead::MethodEnum)i;
                    return succeed;
                }
            }
            return invalid_argument;
        }
    }
}

namespace util
{
    namespace protocol
    {

        HttpPacket & HttpPacket::operator=(
            HttpPacket const & r)
        {
            // head_²»±ä
            data_.reset();
            memcpy(
                boost::asio::buffer_cast<void *>(data_.prepare(r.data_.size())), 
                boost::asio::buffer_cast<void const *>(r.data_.data()), 
                r.data_.size());
            data_.commit(r.data_.size());
            return *this;
        }

        std::string const HttpRequestHead::method_str[] = {
            "GET", 
            "POST", 
            "PUT", 
            "HEAD", 
            "invalid_method"
        };

        bool HttpRequestHead::get_line(
            std::string & line) const
        {
            line = method_str[method];
            line += " ";
            line += path;
            line += " HTTP/";
            line += format(version >> 8);
            line += ".";
            line += format(version & 0xff);
            return true;
        }

        bool HttpRequestHead::set_line(
            std::string const & line)
        {
            std::string str;
            std::istringstream head_stream(line);
            head_stream >> str;
            size_t i = 0;
            for (; i < invalid_method; ++i) {
                if (method_str[i] == str) {
                    break;
                }
            }
            method = (MethodEnum)i;
            head_stream.ignore(1);
            head_stream >> path;
            head_stream.ignore(1);
            int tmp;
            head_stream.ignore(5);
            head_stream >> tmp;
            version = tmp << 8;
            head_stream.ignore(1);
            head_stream >> tmp;
            version |= tmp & 0xff;
            return (bool)head_stream;
        }

        bool HttpResponseHead::get_line(
            std::string & line) const
        {
            line = "HTTP/";
            line += format(version >> 8);
            line += ".";
            line += format(version & 0xff);
            line += " ";
            line += format(err_code);
            line += " ";
            line += err_msg;
            return true;
        }

        bool HttpResponseHead::set_line(
            std::string const & line)
        {
            std::istringstream head_stream(line);
            head_stream.ignore(4); // HTTP/1.0
            int tmp;
            if (head_stream.get() == '/') {
                head_stream >> tmp;
                version = tmp << 8;
                head_stream.ignore(1);
                head_stream >> tmp;
                version |= tmp & 0xff;
                head_stream.ignore(1);
            }
            head_stream >> err_code;
            head_stream.ignore(1);
            std::getline(head_stream, err_msg);
            return (bool)head_stream;
        }

    } // namespace protocol
} // namespace util
