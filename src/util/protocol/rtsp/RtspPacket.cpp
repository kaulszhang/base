// RtspPacket.cpp

#include "util/Util.h"
#include "util/protocol/rtsp/RtspPacket.h"
#include "util/protocol/rtsp/RtspError.h"
#include "util/protocol/rtsp/RtspRequest.h"
#include "util/protocol/rtsp/RtspResponse.h"

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
            util::protocol::RtspRequestHead::MethodEnum const & e)
        {
            return util::protocol::RtspRequestHead::method_str[e];
        }

        error_code from_string(
            util::protocol::RtspRequestHead::MethodEnum & e, 
            std::string const & str)
        {
            for (int i = 0; i < util::protocol::RtspRequestHead::invalid_method; ++i) {
                if (util::protocol::RtspRequestHead::method_str[i] == str) {
                    e = (util::protocol::RtspRequestHead::MethodEnum)i;
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

        RtspPacket & RtspPacket::operator=(
            RtspPacket const & r)
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

        std::string const RtspRequestHead::method_str[] = {
            "DESCRIBE", 
            "ANNOUNCE", 
            "GET_PARAMETER", 
            "SET_PARAMETER", 
            "OPTIONS", 
            "REDIRECT", 
            "SETUP", 
            "PLAY", 
            "RECORD", 
            "PAUSE", 
            "TEARDOWN", 
            "invalid_method"
        };

        bool RtspRequestHead::get_line(
            std::string & line) const
        {
            line = method_str[method];
            line += " ";
            line += path;
            line += " RTSP/";
            line += format(version >> 8);
            line += ".";
            line += format(version & 0xff);
            return true;
        }

        bool RtspRequestHead::set_line(
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

        bool RtspResponseHead::get_line(
            std::string & line) const
        {
            line = "RTSP/";
            line += format(version >> 8);
            line += ".";
            line += format(version & 0xff);
            line += " ";
            line += format(err_code);
            line += " ";
            line += err_msg;
            return true;
        }

        bool RtspResponseHead::set_line(
            std::string const & line)
        {
            std::istringstream head_stream(line);
            head_stream.ignore(4); // RTSP/1.0
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
