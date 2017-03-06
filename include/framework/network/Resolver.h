// Resolver.h
#ifndef _FRAMEWORK_NETWORK_RESOLVER_H_
#define _FRAMEWORK_NETWORK_RESOLVER_H_

#include <framework/network/ResolverService.h>
#include <framework/network/ResolverIterator.h>

#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>

namespace framework
{
    namespace network
    {

        class Resolver
        {
        public:
            typedef ResolverService::implementation_type implementation_type;

        public:
            Resolver(
                boost::asio::io_service & io_svc)
                : service_impl_(boost::asio::use_service<ResolverService>(io_svc))
            {
                service_impl_.construct(impl_);
            }

            ~Resolver()
            {
                service_impl_.destroy(impl_);
            }

            void get_resolvered_host_list(std::string& hosts)
            {
                service_impl_.get_resolvered_host_list(hosts);
            }

            ResolverIterator resolve(
                NetName const & name,
                boost::system::error_code & ec)
            {
                return service_impl_.resolve(impl_, name, ec);
            }

            template <
                typename Handler
            >
            void async_resolve(
                NetName const & name,
                Handler handler)
            {
                service_impl_.async_resolve(impl_, name, handler);
            }

            boost::system::error_code cancel(
                boost::system::error_code & ec)
            {
                return service_impl_.cancel(impl_, ec);
            }

            void cancel()
            {
                boost::system::error_code ec;
                service_impl_.cancel(impl_, ec);
                boost::asio::detail::throw_error(ec);
            }

        private:
            ResolverService & service_impl_;
            implementation_type impl_;
        };
    }
}

#endif // End of _FRAMEWORK_NETWORK_RESOLVER_H_
