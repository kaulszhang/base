// MmsServerManager.h

#ifndef _UTIL_PROTOCOL_MMS_SERVER_MANAGER_H_
#define _UTIL_PROTOCOL_MMS_SERVER_MANAGER_H_

#include <boost/bind.hpp>

#include <framework/network/NetName.h>
#include <framework/network/Acceptor.h>

#include <boost/asio/ip/tcp.hpp>

namespace util
{
    namespace protocol
    {

        struct DefaultMmsServerManager;

        template <
            typename MmsServer, 
            typename Manager = DefaultMmsServerManager
        >
        class MmsServerManager
        {
        public:
            MmsServerManager(
                boost::asio::io_service & io_svc) 
                : acceptor_(io_svc)
                , server_(NULL)
            {
            }

            MmsServerManager(
                boost::asio::io_service & io_svc, 
                framework::network::NetName const & addr)
                : acceptor_(io_svc)
                , addr_(addr)
                , server_(NULL)
            {
            }

            void start()
            {
                server_ = create(this, (Manager *)NULL);
                server_->async_accept(addr_, acceptor_, 
                    boost::bind(&MmsServerManager::handle_accept_client, this, _1));
            }

            boost::system::error_code start(
                framework::network::NetName const & addr, 
                boost::system::error_code & ec)
            {
                if (!framework::network::acceptor_open<boost::asio::ip::tcp>(acceptor_, addr.endpoint(), ec)) {
                    addr_ = addr;
                    server_ = create(this, (Manager *)NULL);
                    server_->async_accept(addr_, acceptor_, 
                        boost::bind(&MmsServerManager::handle_accept_client, this, _1));
                }
                return ec;
            }

            void stop()
            {
                boost::system::error_code ec;
                acceptor_.close(ec);
            }

        public:
            boost::asio::io_service & io_svc()
            {
                return acceptor_.get_io_service();
            }

        private:
            static MmsServer * create(
                MmsServerManager * mgr, 
                MmsServerManager * mgr2)
            {
                return new MmsServer(static_cast<Manager &>(*mgr));
            }

            static MmsServer * create(
                MmsServerManager * mgr, 
                DefaultMmsServerManager * mgr2)
            {
                return new MmsServer(mgr->io_svc());
            }

        private:
            void handle_accept_client(
                boost::system::error_code const & ec)
            {
                if (!ec) {
                    server_->start();
                    server_ = create(this, (Manager *)NULL);
                    server_->async_accept(addr_, acceptor_, 
                        boost::bind(&MmsServerManager::handle_accept_client, this, _1));
                } else {
                    server_->on_error(ec);
                    delete server_;
                }
            }

        private:
            boost::asio::ip::tcp::acceptor acceptor_;
            framework::network::NetName addr_;
            MmsServer * server_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMS_SERVER_MANAGER_H_
