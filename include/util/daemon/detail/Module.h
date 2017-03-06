// Module.h

#ifndef _UTIL_DAEMON_DETAIL_MODULE_H_
#define _UTIL_DAEMON_DETAIL_MODULE_H_

#include "util/daemon/Daemon.h"
#include "util/daemon/detail/ModuleId.h"

namespace util
{
    namespace daemon
    {
        namespace detail
        {

            class Module
            {
            protected:
                Module(
                    Daemon & daemon, 
                    std::string const & name = "")
                    : daemon_(daemon)
                    , name_(name)
                    , id_(NULL)
                    , next_(NULL)
                    , prev_(NULL)
                {
                }

                virtual ~Module()
                {
                }

            public:
                Daemon & get_daemon()
                {
                    return daemon_;
                }

                boost::asio::io_service & io_svc()
                {
                    return daemon_.io_svc();
                }

                framework::configure::Config & config()
                {
                    return daemon_.config();
                }

                framework::logger::Logger & logger()
                {
                    return daemon_.logger();
                }

                std::string const & name() const
                {
                    return name_;
                }

            private:
                virtual boost::system::error_code startup() = 0;

                virtual void shutdown() = 0;

                virtual boost::system::error_code resume()
                {
                    return boost::system::error_code();
                }

                virtual void pause()
                {
                };

                friend class detail::ModuleRegistry;

                util::daemon::Daemon & daemon_;
                std::string const name_;
                util::daemon::detail::Id const * id_;
                util::daemon::detail::Module * next_;
                util::daemon::detail::Module * prev_;
            };

        } // namespace detail
    } // namespace daemon
} // namespace util

#endif // _UTIL_DAEMON_MODULE_H_
