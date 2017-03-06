// NameCache.h

#include <framework/network/NetName.h>
#include <framework/network/Endpoint.h>

#include <boost/shared_ptr.hpp>
#include <boost/asio/io_service.hpp>

namespace framework
{
    namespace network
    {

        struct ResolveTask;
        class ResolverIterator;
        class ResolverServiceImpl;

        class ResolveHandler
        {
        protected:
            typedef void (*handle_type)(
                ResolveHandler & handle, 
                boost::system::error_code const & ec, 
                ResolverIterator const & iterator);

            typedef void (*destroy_type)(
                ResolveHandler & handle);

            ResolveHandler(
                boost::asio::io_service & io_svc, 
                handle_type handle, 
                destroy_type destroy)
                : work_(io_svc)
                , handle_(handle)
                , destroy_(destroy)
            {
            }

            ~ResolveHandler()
            {
            }

        public:
            void handle(
                boost::system::error_code const & ec, 
                ResolverIterator const & iterator)
            {
                handle_(*this, ec, iterator);
                destroy_(*this);
            }

        private:
            boost::asio::io_service::work work_;
            handle_type handle_;
            destroy_type destroy_;
        };

        template <typename Handler>
        class ResolveHandlerT
            : public ResolveHandler
        {
        public:
            ResolveHandlerT(
                boost::asio::io_service & io_svc, 
                Handler const & handler)
                : ResolveHandler(io_svc, &ResolveHandlerT::handle, &ResolveHandlerT::destroy)
                , handler_(handler)
            {
            }

        private:
            static void handle(
                ResolveHandler & handler, 
                boost::system::error_code const & ec, 
                ResolverIterator const & iterator)
            {
                ResolveHandlerT & me = static_cast<ResolveHandlerT &>(handler);
                return me.handler_(ec, iterator);
            }

            static void destroy(
                ResolveHandler & handler)
            {
                ResolveHandlerT & me = static_cast<ResolveHandlerT &>(handler);
                delete &me;
            }

        private:
            Handler handler_;
        };

        class ResolverService
            : public boost::asio::detail::service_base<ResolverService>
        {
        public:
            typedef boost::shared_ptr<ResolveTask> implementation_type;

        public:
            ResolverService(
                boost::asio::io_service & io_svc);

            ~ResolverService();

            void shutdown_service();

        public:
            void construct(
                implementation_type & impl);

            void destroy(
                implementation_type & impl);

            boost::system::error_code cancel(
                implementation_type & impl, 
                boost::system::error_code & ec);

            ResolverIterator resolve(
                implementation_type & impl,
                NetName const & name,
                boost::system::error_code & ec);

            template <
                typename Handler
            >
            void async_resolve(
                implementation_type & impl,
                NetName const & name,
                Handler handler)
            {
                pri_async_resolve(impl, name, 
                    new ResolveHandlerT<Handler>(get_io_service(), handler));
            }

        public:
            void insert_name(
                NetName const & ,
                std::vector<Endpoint> const &);

            void get_resolvered_host_list(std::string& hosts);

        private:
            friend class ResolverIterator;
            void increment(
                ResolverIterator & iter);

        private:
            void pri_async_resolve(
                implementation_type & impl,
                NetName const & name,
                ResolveHandler * handler);

            friend class ResolverServiceImpl;
            void call_back(
                ResolveHandler * handler, 
                boost::system::error_code const & ec, 
                ResolverIterator const & iter);

        private:
            ResolverServiceImpl * service_impl_;
        };

    } // namespace network
} // namespace framework
