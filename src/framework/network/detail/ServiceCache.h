// HostCache.h

#ifdef FRAMEWORK_NETWORK_WITH_SERVICE_CACHE
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
using namespace boost::asio::ip;
#endif

namespace framework
{
    namespace network
    {

        namespace detail
        {

            class ServiceCache
            {
            public:
                ServiceCache(
                    boost::asio::io_service & io_svc)
#ifdef FRAMEWORK_NETWORK_WITH_SERVICE_CACHE
                    : tcp_resolver_(io_svc)
                    , udp_resolver_(io_svc)
#endif
                {
#ifndef FRAMEWORK_NETWORK_WITH_SERVICE_CACHE
                    cache_["ftp"] = Endpoint(Endpoint::tcp, 21);
                    cache_["http"] = Endpoint(Endpoint::tcp, 80);
#endif
                }

            public:
                boost::system::error_code find(
                    NetName const & name, 
                    Endpoint & endpoint, 
                    boost::system::error_code & ec)
                {
#ifndef FRAMEWORK_NETWORK_WITH_SERVICE_CACHE
                    if (name.svc().find_first_not_of("0123456789") == std::string::npos) {
                        endpoint.protocol(Endpoint::unspec_protocol);
                        boost::uint32_t port;
                        if (framework::string::parse2(name.svc(), port)) {
                            return ec = boost::asio::error::service_not_found;
                        } else {
                            endpoint.protocol((Endpoint::ProtocolEnum)name.protocol());
                            endpoint.port(port);
                            ec.clear();
                            return ec;
                        }
                    }
                    std::map<std::string, Endpoint>::const_iterator iter = cache_.find(name.svc());
                    if (iter == cache_.end()) {
                        return ec = boost::asio::error::service_not_found;
                    }
                    if (name.protocol() != NetName::unspec_protocol
                        && (int)iter->second.protocol() != (int)name.protocol()) {
                            return ec = boost::asio::error::service_not_found;
                    }
                    endpoint = iter->second;
                    ec.clear();
                    return ec;
#else
                    if (name.protocol() == NetName::tcp) {
                        tcp::resolver::iterator iter = tcp_resolver_.resolve(
                            tcp::resolver::query(tcp::v4(), name.svc().c_str()), ec);
                        if (!ec) {
                            endpoint = Endpoint(Endpoint::tcp, (*iter).endpoint().port());
                        }
                    } else if (name.protocol() == NetName::udp) {
                        udp::resolver::iterator iter = udp_resolver_.resolve(
                            udp::resolver::query(udp::v4(), name.svc().c_str()), ec);
                        if (!ec) {
                            endpoint = Endpoint(Endpoint::udp, (*iter).endpoint().port());
                        }
                    } else {
                        tcp::resolver::iterator iter = tcp_resolver_.resolve(
                            tcp::resolver::query(tcp::v4(), name.svc().c_str()), ec);
                        if (!ec) {
                            endpoint = Endpoint(Endpoint::tcp, (*iter).endpoint().port());
                        } else {
                            udp::resolver::iterator iter = udp_resolver_.resolve(
                                udp::resolver::query(udp::v4(), name.svc().c_str()), ec);
                            if (!ec) {
                                endpoint = Endpoint(Endpoint::udp, (*iter).endpoint().port());
                            }
                        }
                    }
                    return ec;
#endif
                }

            private:
#ifndef FRAMEWORK_NETWORK_WITH_SERVICE_CACHE
                std::map<std::string, Endpoint> cache_;
#else
                tcp::resolver tcp_resolver_;
                udp::resolver udp_resolver_;
#endif
            };

        } // namespace detail

    } // namespace network
} // namespace framework
