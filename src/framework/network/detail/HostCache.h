// HostCache.h

#define INITLEFTTIME   300

#include "framework/filesystem/Path.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <fstream>

namespace framework
{
    namespace network
    {

        namespace detail
        {

            class HostCache
            {
            public:
                struct CacheData
                {
                    enum StatusEnum
                    {
                        init, // temp status in load loop
                        weak, // not responsible
                        global, // from dns server
                    };

                    CacheData()
                        : left_times(0)
                        , status(init)
                    {
                    }

                    std::vector<Endpoint> v4_endpoints;
                    std::vector<Endpoint> v6_endpoints;
                    size_t left_times;
                    StatusEnum status;
                };

            public:
                HostCache(
                    boost::asio::io_service & io_svc)
                    : tcp_resolver_(io_svc)
                    , udp_resolver_(io_svc)
                {
                    //load();
                    //save_time_ += framework::timer::Duration::seconds(60);
                }

                ~HostCache()
                {
                    //save_safe();
                }

            public:
                bool find(
                    NetName const & name, 
                    std::vector<Endpoint> & endpoints)
                {
                    endpoints.clear();
                    std::map<std::string, CacheData>::const_iterator iter = cache_.find(name.host());
                    if (iter == cache_.end()) {
                        return false;
                    } else {
                        CacheData const & data = iter->second;
                        if (name.family() == NetName::unspec_family) {
                            endpoints = data.v4_endpoints;
                            endpoints.insert(endpoints.end(), 
                                data.v6_endpoints.begin(), data.v6_endpoints.end());
                        } else if (name.family() == NetName::v4) {
                            endpoints = data.v4_endpoints;
                        } else {
                            endpoints = data.v6_endpoints;
                        }
                        if (name.protocol() == NetName::unspec_protocol) {
                            size_t n = endpoints.size();
                            for (size_t i = 0; i < n; ++i) {
                                endpoints[i].protocol(Endpoint::udp);
                                endpoints.push_back(endpoints[i]);
                                endpoints[i].protocol(Endpoint::tcp);
                            }
                        } else {
                            for (size_t i = 0; i < endpoints.size(); ++i) {
                                endpoints[i].protocol(Endpoint::ProtocolEnum(name.protocol()));
                            }
                        }
                        return !endpoints.empty();
                    }
                }

                void check(
                    std::vector<NetName> & expired_hosts)
                {
                    std::map<std::string, CacheData>::iterator iter = cache_.begin();
                    for (; iter != cache_.end(); ++iter) {
                        CacheData & data = iter->second;
                        if (data.left_times > 0 && --data.left_times == 0) {
                            //(*iter).left_times = INITLEFTTIME;
                            NetName name;
                            name.host((*iter).first);
                            expired_hosts.push_back(name);
                        }
                    }
                }

                boost::system::error_code resolve(
                    NetName const & name, 
                    std::vector<Endpoint> & endpoints,
                    boost::system::error_code & ec)
                {
                    boost::system::error_code ec1;
                    boost::system::error_code ec2;
                    if (name.protocol() == NetName::unspec_protocol || name.protocol() == NetName::tcp) {
                        boost::asio::ip::tcp::resolver::iterator iter;
#ifdef FRAMEWORK_NETWORK_HOST_V6_BUG
                        if (name.family() == NetName::v6) {
                            ec2 = boost::asio::error::host_not_found;
                        } else {
                            iter = tcp_resolver_.resolve(
                                boost::asio::ip::tcp::resolver::query(boost::asio::ip::tcp::v4(), name.host().c_str(), name.svc().c_str()), ec1);
                        }
#else
                        if (name.family() == NetName::v6) {
                            iter = tcp_resolver_.resolve(
                                boost::asio::ip::tcp::resolver::query(boost::asio::ip::tcp::v6(), name.host().c_str(), name.svc().c_str()), ec1);
                        } else if (name.family() == NetName::v4) {
                            iter = tcp_resolver_.resolve(
                                boost::asio::ip::tcp::resolver::query(boost::asio::ip::tcp::v4(), name.host().c_str(), name.svc().c_str()), ec1);
                        } else {
                            iter = tcp_resolver_.resolve(
                                boost::asio::ip::tcp::resolver::query(name.host().c_str(), name.svc().c_str()), ec1);
                        }
#endif
                        boost::asio::ip::tcp::resolver::iterator end;
                        if (!ec1) {
                            while (iter != end) {
                                Endpoint end_point((*iter++).endpoint());
                                endpoints.push_back(end_point);
                            }
                        }
                    } else {
                        ec1 = boost::asio::error::service_not_found;
                    }
                    if (name.protocol() == NetName::unspec_protocol || name.protocol() == NetName::udp) {
                        std::vector<Endpoint> udp_endpoints;
                        boost::asio::ip::udp::resolver::iterator iter;
#ifdef FRAMEWORK_NETWORK_HOST_V6_BUG
                        if (name.family() == NetName::v6) {
                            ec2 = boost::asio::error::host_not_found;
                        } else {
                            iter = udp_resolver_.resolve(
                                boost::asio::ip::udp::resolver::query(boost::asio::ip::udp::v4(), name.host().c_str(), name.svc().c_str()), ec2);
                        }
#else
                        if (name.family() == NetName::v6) {
                            iter = udp_resolver_.resolve(
                                boost::asio::ip::udp::resolver::query(boost::asio::ip::udp::v6(), name.host().c_str(), name.svc().c_str()), ec2);
                        } else if (name.family() == NetName::v4) {
                            iter = udp_resolver_.resolve(
                                boost::asio::ip::udp::resolver::query(boost::asio::ip::udp::v4(), name.host().c_str(), name.svc().c_str()), ec2);
                        } else {
                            iter = udp_resolver_.resolve(
                                boost::asio::ip::udp::resolver::query(name.host().c_str(), name.svc().c_str()), ec2);
                        }
#endif
                        boost::asio::ip::udp::resolver::iterator end;
                        if (!ec2) {
                            while (iter != end) {
                                Endpoint end_point((*iter++).endpoint());
                                udp_endpoints.push_back(end_point);
                            }
                            endpoints.insert(endpoints.end(), udp_endpoints.begin(), udp_endpoints.end());
                        }
                    } else {
                        ec2 = boost::asio::error::service_not_found;
                    }
                    if (ec1 == boost::asio::error::service_not_found) {
                        ec = ec2;
                    } else {
                        ec = ec1;
                    }
                    return ec;
                }

                void get_resolvered_host_list(std::string& hosts)
                {
                    hosts.clear();
                    std::map<std::string, CacheData>::iterator iter = cache_.begin();
                    for (; iter != cache_.end(); ++iter)
                    {
                        if (iter != cache_.begin())
                            hosts += "|";
                        hosts += iter->first;
                        hosts += "=";
                        CacheData & data = iter->second;
                        if (!data.v4_endpoints.empty())
                        {
                            hosts += data.v4_endpoints[0].ip_str();
                        }
                        else if (!data.v6_endpoints.empty())
                        {
                            hosts += data.v6_endpoints[0].ip_str();
                        }
                        else
                        {
                            assert(false);
                            hosts += iter->first;
                        }
                    }
                }

                void update(
                    NetName const & name, 
                    std::vector<Endpoint> const & endpoints)
                {
                    // 更新缓存
                    std::map<std::string, CacheData>::iterator iter = cache_.find(name.host());
                    if (iter == cache_.end()) {
                        iter = cache_.insert(std::make_pair(name.host(), CacheData())).first;
                    }
                    CacheData & data = iter->second;
                    if (name.family() == NetName::unspec_family) {
                        data.v4_endpoints.clear();
                        data.v6_endpoints.clear();
                        for (size_t i = 0; i < endpoints.size(); ++i) {
                            if (endpoints[i].family() == Endpoint::v4) {
                                data.v4_endpoints.push_back(endpoints[i]);
                            } else {
                                data.v6_endpoints.push_back(endpoints[i]);
                            }
                        }
                    } else if (name.family() == NetName::v4) {
                        data.v4_endpoints = endpoints;
                    } else {
                        data.v6_endpoints = endpoints;
                    }
                    for (size_t i = 0; i < data.v4_endpoints.size(); ++i) {
                        data.v4_endpoints[i].protocol(Endpoint::unspec_protocol);
                        data.v4_endpoints[i].port(0);
                    }
                    for (size_t i = 0; i < data.v6_endpoints.size(); ++i) {
                        data.v6_endpoints[i].protocol(Endpoint::unspec_protocol);
                        data.v6_endpoints[i].port(0);
                    }
                    data.status = CacheData::global;
                    data.left_times = INITLEFTTIME;

                    /*if (framework::timer::Time::now() > save_time_) {
                        save_safe();
                        save_time_ = framework::timer::Time::now();
                        save_time_ += framework::timer::Duration::seconds(60);
                    }*/
                }

                void update_weak( // update only if not exists
                    NetName const & name, 
                    std::vector<Endpoint> const & endpoints)
                {
                    std::map<std::string, CacheData>::iterator iter = cache_.find(name.host());
                    if (iter == cache_.end()) {
                        iter = cache_.insert(std::make_pair(name.host(), CacheData())).first;
                        CacheData & data = iter->second;
                        for (size_t i = 0; i < endpoints.size(); ++i) {
                            if (endpoints[i].family() == Endpoint::v4) {
                                data.v4_endpoints.push_back(endpoints[i]);
                            } else {
                                data.v6_endpoints.push_back(endpoints[i]);
                            }
                        }
                    }
                }

                void save_safe()
                {
                    load();
                    boost::system::error_code ec;
                    boost::filesystem::remove(cache_file(), ec);
                    save();
                }

            private:
                static boost::filesystem::path cache_file()
                {
                    return (framework::filesystem::framework_temp_path() / "hosts");
                }

                void load()
                {
                    std::ifstream ifs;
                    ifs.open(cache_file().string().c_str());
                    std::string line;
                    while (std::getline(ifs, line)) {
                        if (line.empty() || line[0] == '#') {
                            continue;
                        }
                        std::string::size_type p = line.find('\t');
                        if (p == std::string::npos) {
                            continue;
                        }
                        Endpoint ep(line.substr(0, p));
                        if (ep.family() == Endpoint::unspec_family) {
                            continue;
                        }
                        CacheData & data = cache_[line.substr(p + 1)];
                        // 已经有了，不更新
                        if (data.status == CacheData::global) {
                            continue;
                        } else if (data.status == CacheData::weak) {
                            data.v4_endpoints.clear();
                            data.v6_endpoints.clear();
                            data.status = CacheData::init;
                        }
                        if (ep.family() == Endpoint::v4) {
                            if (std::find(data.v4_endpoints.begin(), data.v4_endpoints.end(), ep) == data.v4_endpoints.end())
                                data.v4_endpoints.push_back(ep);
                        } else {
                            if (std::find(data.v6_endpoints.begin(), data.v6_endpoints.end(), ep) == data.v6_endpoints.end())
                                data.v6_endpoints.push_back(ep);
                        }
                    }
                    std::map<std::string, CacheData>::iterator iter = cache_.begin();
                    for (; iter != cache_.end(); ++iter) {
                        CacheData & data = iter->second;
                        if (data.status == CacheData::init) {
                            data.status = CacheData::weak;
                        }
                    }
                }

                void save() const
                {
                    std::ofstream ofs;
                    ofs.open(cache_file().string().c_str());
                    std::map<std::string, CacheData>::const_iterator iter = cache_.begin();
                    for (; iter != cache_.end(); ++iter) {
                        CacheData const & data = iter->second;
                        for (size_t i = 0; i < data.v4_endpoints.size(); ++i) {
                            ofs << data.v4_endpoints[i].ip_str() << "\t" << iter->first << std::endl;
                        }
                        for (size_t i = 0; i < data.v6_endpoints.size(); ++i) {
                            ofs << data.v6_endpoints[i].ip_str() << "\t" << iter->first << std::endl;
                        }
                    }
                }

            private:
                boost::asio::ip::tcp::resolver tcp_resolver_;
                boost::asio::ip::udp::resolver udp_resolver_;
                framework::timer::Time save_time_;
                std::map<std::string, CacheData> cache_;
            };

        } // namespace detail

    } // namespace network
} // namespace framework
