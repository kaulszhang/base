// NetName.cpp

#include "framework/Framework.h"
#include "framework/system/BytesOrder.h"
#include "framework/network/NetName.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
#include "framework/string/StringToken.h"
using namespace framework::system;
using namespace framework::system::logic_error;
using namespace framework::string;

#ifndef BOOST_WINDOWS_API
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <arpa/inet.h>
#else
#  include <winsock2.h>
#endif

using namespace boost::system;

namespace framework
{
    namespace network
    {

        NetName::NetName(
            std::string const & host, 
            std::string const & svc)
            : protocol_(unspec_protocol)
            , family_(unspec_family)
            , host_(host)
            , svc_(svc)
        {
        }

        NetName::NetName(
            std::string const & host, 
            int port)
            : protocol_(unspec_protocol)
            , family_(unspec_family)
            , host_(host)
            , svc_(format(port))
        {
        }

        NetName::NetName(
            std::string const & host,
            int port,
            std::string const & app_protocol)
            : protocol_(unspec_protocol)
            , family_(unspec_family)
            , host_(host)
            , svc_(format(port))
            , app_protocol_(app_protocol)
        {
            if (port == 0)
            {
                svc_.clear();
                /*if (app_protocol_ == "http")
                    svc_ = format(80);
                else if(app_protocol_ == "https")
                    svc_ = format(443);
                else if (app_protocol_ == "rtmp")
                    svc_ = format(1935);
                else
                {
                    svc_ = format(80);
                    assert(0);
                }*/
            }
        }



        NetName::NetName(
            std::string const & str)
            : protocol_(unspec_protocol)
            , family_(unspec_family)
        {
            from_string(str);
        }

        NetName::NetName(
            FamilyEnum f, 
            std::string const & host, 
            std::string const & svc)
            : protocol_(unspec_protocol)
            , family_(f)
            , host_(host)
            , svc_(svc)
        {
        }

        NetName::NetName(
            FamilyEnum f, 
            std::string const & host, 
            int port)
            : protocol_(unspec_protocol)
            , family_(f)
            , host_(host)
            , svc_(format(port))
        {
        }

        NetName::NetName(
            FamilyEnum f, 
            std::string const & str)
            : protocol_(unspec_protocol)
            , family_(f)
        {
            from_string(str);
        }

        NetName::NetName(
            ProtocolEnum p, 
            std::string const & host, 
            std::string const & svc)
            : protocol_(p)
            , family_(unspec_family)
            , host_(host)
            , svc_(svc)
        {
        }

        NetName::NetName(
            ProtocolEnum p, 
            std::string const & host, 
            int port)
            : protocol_(p)
            , family_(unspec_family)
            , host_(host)
            , svc_(format(port))
        {
        }

        NetName::NetName(
            ProtocolEnum p, 
            std::string const & str)
            : protocol_(p)
            , family_(unspec_family)
        {
            from_string(str);
        }

        NetName::NetName(
            ProtocolEnum p, 
            FamilyEnum f, 
            std::string const & host, 
            std::string const & svc)
            : protocol_(p)
            , family_(f)
            , host_(host)
            , svc_(svc)
        {
        }

        NetName::NetName(
            ProtocolEnum p, 
            FamilyEnum f, 
            std::string const & host, 
            int port)
            : protocol_(p)
            , family_(f)
            , host_(host)
            , svc_(format(port))
        {
        }

        NetName::NetName(
            ProtocolEnum p, 
            FamilyEnum f, 
            std::string const & str)
            : protocol_(p)
            , family_(f)
        {
            from_string(str);
        }

        unsigned long NetName::ip() const
        {
            return BytesOrder::net_to_host_long(ip_pton(host_));
        }

        unsigned short NetName::port() const
        {
            return parse<unsigned short>(svc_);
        }

        unsigned long NetName::net_ip() const
        {
            return ip_pton(host_);
        }

        unsigned short NetName::net_port() const
        {
            return BytesOrder::host_to_net_short(parse<unsigned short>(svc_));
        }

        bool NetName::is_digit() const
        {
            return (host_.find_first_not_of(".0123456789") == std::string::npos
                || host_.find(':') != std::string::npos) 
                && svc_.find_first_not_of("0123456789") == std::string::npos;
        }

        Endpoint NetName::endpoint() const
        {
            return Endpoint((Endpoint::ProtocolEnum)protocol_, host_, port());
        }

        void NetName::ip(
            unsigned long v)
        {
            host_ = ip_ntop(BytesOrder::host_to_net_long(v));
        }

        void NetName::port(
            unsigned short v)
        {
            svc_ = format(v);
        }

        void NetName::net_ip(
            unsigned long v)
        {
            host_ = ip_ntop(v);
        }

        void NetName::net_port(
            unsigned short v)
        {
            svc_ = format(BytesOrder::net_to_host_short(v));
        }

        std::string NetName::to_string() const
        {
            return std::string((protocol_ == tcp ? "(tcp)" : (protocol_ == udp ? "(udp)" : ""))) 
                + (family_ == v4 ? "(v4)" : (family_ == v6 ? "(v6)[" : "")) 
                + host_ 
                + (family_ == v6 ? "]:" : ":")
                + svc_;
        }

        error_code NetName::from_string(
            std::string const & str)
        {
            std::string::size_type beg = 0;
            if (str.compare(beg, 5, "(tcp)", 5) == 0) {
                protocol_ = tcp;
                beg += 5;
            } else if (str.compare(beg, 5, "(udp)", 5) == 0) {
                protocol_ = udp;
                beg += 5;
            }
            if (str.compare(beg, 4, "(v4)", 4) == 0) {
                family_ = v4;
                beg += 4;
            } else if (str.compare(beg, 4, "(v6)", 4) == 0) {
                family_ = v6;
                beg += 4;
            }


            error_code ec(invalid_argument);
            if (str.at(beg) == '[')
            {
                family_ = v6;
                std::string::size_type index = std::string::npos;
                index = str.find(']');
                if (index != std::string::npos)
                {
                    host_ = str.substr(1, index-1);
                    std::string::size_type indexend = str.find(':', index);
                    if (indexend != std::string::npos)
                    {
                        svc_ = str.substr(indexend + 1, str.size());
                    }
                    ec = succeed;
                }
            }
            else
            {

                StringToken st(str, ":", false, beg);
                std::string h;
                
                if (!st.next_token(h, ec)) {
                    if (!h.empty()) {
                        host_ = h;
                    }
                    if (!st.remain().empty()) {
                        svc_ = st.remain();
                    }
                    ec = succeed;
                }
            }
            return ec;
        }

        std::string NetName::ip_ntop(
            unsigned long ip)
        {
    #ifndef WIN32
            char buf[64];
            inet_ntop(AF_INET, &ip, buf, 64);
            return std::string(buf);
    #else
            in_addr in;
            in.S_un.S_addr = ip;
            return std::string(inet_ntoa(in));
    #endif
        }

        unsigned long NetName::ip_pton(
            std::string const & ip)
        {
    #ifndef WIN32
            unsigned long n;
            inet_pton(AF_INET, ip.c_str(), &n);
            return n;
    #else
            unsigned long n = inet_addr(ip.c_str());
            return n == INADDR_NONE ? 0 : n;
    #endif
        }

    } // namespace network
} // namespace framework
