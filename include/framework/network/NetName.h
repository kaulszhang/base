// NetName.h

/** 定义了一个基本的地址结构和IP地址转换函数以及一些字节顺转换函数
*/

#ifndef _FRAMEWORK_NETWORK_NET_NAME_H_
#define _FRAMEWORK_NETWORK_NET_NAME_H_

#include "framework/network/Endpoint.h"

namespace framework
{
    namespace network
    {

        class NetName
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

        public:
            NetName(
                ProtocolEnum p, 
                FamilyEnum f = unspec_family)
                : protocol_(p)
                , family_(f)
            {
            }

            NetName(
                FamilyEnum f = unspec_family)
                : protocol_(unspec_protocol)
                , family_(f)
            {
            }

            NetName(
                ProtocolEnum p, 
                NetName const & r)
                : protocol_(p)
                , family_(r.family_)
                , host_(r.host_)
                , svc_(r.svc_)
                , app_protocol_(r.app_protocol_)
            {
            }

            NetName(
                std::string const & host, 
                int port);

            NetName(
                std::string const & host,
                int port,
                std::string const & app_protocol);

            NetName(
                std::string const & host, 
                std::string const & svc);

            NetName(
                std::string const & str);

            NetName(
                ProtocolEnum p, 
                std::string const & host, 
                int port);

            NetName(
                ProtocolEnum p, 
                std::string const & host, 
                std::string const & svc);

            NetName(
                ProtocolEnum p, 
                std::string const & str);

            NetName(
                FamilyEnum f, 
                std::string const & host, 
                int port);

            NetName(
                FamilyEnum f, 
                std::string const & host, 
                std::string const & svc);

            NetName(
                FamilyEnum f, 
                std::string const & str);

            NetName(
                ProtocolEnum p, 
                FamilyEnum f, 
                std::string const & host, 
                int port);

            NetName(
                ProtocolEnum p, 
                FamilyEnum f, 
                std::string const & host, 
                std::string const & svc);

            NetName(
                ProtocolEnum p, 
                FamilyEnum f, 
                std::string const & str);

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

            std::string const & app_protocol() const
            {
                return app_protocol_;
            }

            void app_protocol(std::string const & p)
            {
                app_protocol_ = p;
            }

            std::string const & host() const
            {
                return host_;
            }

            void host(
                std::string const & h)
            {
                host_ = h;
            }

            std::string const & svc() const
            {
                return svc_;
            }

            void svc(
                std::string const & s)
            {
                svc_ = s;
            }

            std::string host_svc() const
            {
                std::string hs = host_;

                if (family_ == v6)
                {
                    hs = "[" + host_ + "]";
                }

                if (!svc_.empty()) {
                    hs.append(1, ':');
                    hs.append(svc_);
                }
                return hs;
            }

        public:
            unsigned long ip() const;

            unsigned short port() const;

            unsigned long net_ip() const;

            unsigned short net_port() const;

            bool is_digit() const;

            Endpoint endpoint() const;

        public:
            void ip(
                unsigned long v);

            void port(
                unsigned short v);

            void net_ip(
                unsigned long v);

            void net_port(
                unsigned short v);

        public:
            std::string to_string() const;

            boost::system::error_code from_string(
                std::string const & str);

        public:
            friend bool operator==(
                NetName const & l, 
                NetName const & r)
            {
                return l.protocol_ == r.protocol_ 
                    && l.family_ == r.family_ 
                    && l.host_ == r.host_ 
                    && l.svc_ == r.svc_
                    && l.app_protocol_ == r.app_protocol_;
            }

            friend bool operator!=(
                NetName const & l, 
                NetName const & r)
            {
                return !(l == r);
            }

        public:
            static std::string ip_ntop(
                unsigned long ip);

            static unsigned long ip_pton(
                std::string const & ip);

        private:
            ProtocolEnum protocol_;
            FamilyEnum family_;
            std::string host_;	// IP字符串,也可以是域名DN
            std::string svc_;	// 端口
            std::string app_protocol_; //应用层网络协议
        };

    } // namespace network
} // namespace framework

#endif // _FRAMEWORK_NETWORK_NET_NAME_H_
