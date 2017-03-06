// Endpoint.h

#ifndef _FRAMEWORK_NETWORK_ENDPOINT_H_
#define _FRAMEWORK_NETWORK_ENDPOINT_H_

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace framework
{
    namespace network
    {

        class NetName;

        class Endpoint
        {
        public:
            enum ProtocolEnum
            {
                tcp, 
                udp, 
                unspec_protocol, 
            };

            enum FamilyEnum
            {
                v4, 
                v6, 
                unspec_family, 
            };

            Endpoint(
                ProtocolEnum protocol = unspec_protocol)
                : protocol_(protocol)
                , family_(unspec_family)
                , port_(0)
            {
            }

            Endpoint(
                ProtocolEnum protocol, 
                boost::uint16_t port)
                : protocol_(protocol)
                , family_(unspec_family)
                , port_(port)
            {
            }

            Endpoint(
                ProtocolEnum protocol, 
                boost::uint32_t ip_v4, 
                boost::uint16_t port)
                : protocol_(protocol)
                , family_(v4)
                , ip_v4_(ip_v4)
                , port_(port)
            {
            }

            Endpoint(
                boost::uint32_t ip_v4, 
                boost::uint16_t port)
                : protocol_(unspec_protocol)
                , family_(v4)
                , ip_v4_(ip_v4)
                , port_(port)
            {
            }

            Endpoint(
                ProtocolEnum p, 
                Endpoint const & r)
                : protocol_(p)
                , family_(r.family_)
                , port_(r.port_)
            {
                memcpy(ip_v6_, r.ip_v6_, sizeof(ip_v6_));
            }

            Endpoint(
                std::string const & ip_str, 
                boost::uint16_t port);

            Endpoint(
                ProtocolEnum protocol, 
                std::string const & ip_str, 
                boost::uint16_t port);

            Endpoint(
                std::string const & str)
                : protocol_(unspec_protocol)
                , family_(unspec_family)
                , port_(0)
            {
                from_string(str);
            }

        public:
            std::string to_string() const;

            boost::system::error_code from_string(
                std::string const & str);

        public:
            friend bool operator == (
                Endpoint const & l, 
                Endpoint const & r)
            {
                return l.protocol_ == r.protocol_ 
                    && l.family_ == r.family_ 
                    && (l.family_ == v4 ? l.ip_v4_ == r.ip_v4_ : (
                        l.family_ == v6 ? memcmp(l.ip_v6_, r.ip_v6_, sizeof(l.ip_v6_)) == 0 : true))
                    && l.port_ == r.port_;
            }

            friend bool operator != (
                Endpoint const & l, 
                Endpoint const & r)
            {
                return !(l == r);
            }

        public:
            ProtocolEnum protocol() const
            {
                return protocol_;
            }

            void protocol(
                ProtocolEnum p)
            {
                protocol_ = p;
            }

            FamilyEnum family() const
            {
                return family_;
            }

            void family(
                FamilyEnum f)
            {
                family_ = f;
            }

            boost::uint32_t ip_v4() const
            {
                return ip_v4_;
            }

            void ip_v4(
                boost::uint32_t ip_v4)
            {
                ip_v4_ = ip_v4;
            }

            std::string ip_str() const;

            void ip(
                std::string const & h);

            boost::uint16_t port() const
            {
                return port_;
            }

            void port(
                boost::uint16_t port)
            {
                port_ = port;
            }

        public:
            Endpoint(
                boost::asio::ip::udp::endpoint const & end_point);

            Endpoint(
                boost::asio::ip::tcp::endpoint const & end_point);

            operator boost::asio::ip::udp::endpoint() const;

            operator boost::asio::ip::tcp::endpoint() const;

        private:
            ProtocolEnum protocol_;
            FamilyEnum family_;
            union {
                boost::uint32_t ip_v4_;
                boost::uint8_t ip_v6_[16];
            };
            boost::uint16_t port_;
        };

    } // namespace network
} // namespace framework

#endif//_FRAMEWORK_NETWORK_ENDPOINT_H_
