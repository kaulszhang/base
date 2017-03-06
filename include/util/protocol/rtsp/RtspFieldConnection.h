// RtspFieldConnection.h

#ifndef _UTIL_PROTOCOL_RTSP_FIELD_CONNECTION_H_
#define _UTIL_PROTOCOL_RTSP_FIELD_CONNECTION_H_

#include <iterator>
#include <functional>
#include <locale>
#include <algorithm>

namespace util
{
    namespace protocol
    {
        namespace rtsp_field
        {

            class Connection
            {
            public:
                enum TypeEnum
                {
                    keep_alive, 
                    close, 
                };

            public:
                Connection(
                    TypeEnum t)
                    : type_(t)
                {
                }

                bool operator==(
                    Connection const & r)
                {
                    return type_ == r.type_;
                }

                std::string to_string() const
                {
                    if (type_ == keep_alive) {
                        return "Keep-Alive";
                    } else if (type_ == close) {
                        return "Close";
                    } else {
                        assert(0);
                    }
                }

                struct tolower
                {
                public:
                    char operator()(
                        char c)
                    {
                        return std::tolower(c, lc_);
                    }
                private:
                    std::locale lc_;
                };

                boost::system::error_code from_string(
                    std::string const & str)
                {
                    std::string str1;
                    std::transform(str.begin(), str.end(), 
                        std::back_inserter(str1), tolower());
                    if (str1 == "keep-alive") {
                        type_ = keep_alive;
                    } else if (str1 == "close") {
                        type_ = close;
                    } else {
                        return framework::system::logic_error::invalid_argument;
                    }
                    return boost::system::error_code();
                }

            private:
                TypeEnum type_;
            };

        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_FIELD_CONNECTION_H_
