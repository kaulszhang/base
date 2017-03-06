// Daemon.cpp

#include "util/Util.h"
#include "util/daemon/Daemon.h"

#include <framework/logger/LoggerStreamRecord.h>
using framework::logger::Logger;

#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Daemon", 0);

namespace util
{
    namespace daemon
    {

        static inline std::vector<Daemon *> & daemon_set()
        {
            static std::vector<Daemon *> vec;
            return vec;
        }
        
        void Daemon::register_daemon(
            Daemon & daemon)
        {
            daemon_set().push_back(&daemon);
        }

        void Daemon::unregister_daemon(
            Daemon & daemon)
        {
            daemon_set().erase(std::find(daemon_set().begin(), daemon_set().end(), &daemon));
        }

        Daemon & Daemon::from_io_svc(
            boost::asio::io_service & io_svc)
        {
            Daemon * d = (Daemon *)&io_svc;
            d = (Daemon * )((char *)d - ((char *)&d->io_svc_ - (char *)d));
            if (std::find(daemon_set().begin(), daemon_set().end(), d) == daemon_set().end())
                d = NULL;
            return *d;
        }

        Daemon::Daemon()
            : io_work_(NULL)
            , module_registry_(new detail::ModuleRegistry(*this))
        {
            //logger_.load_config(config_);
            register_daemon(*this);
        }

        Daemon::Daemon(
            std::string const & conf)
            : io_work_(NULL)
            , config_(conf)
            , module_registry_(new detail::ModuleRegistry(*this))
        {
            register_daemon(*this);
            //logger_.load_config(config_);
        }

        Daemon::~Daemon()
        {
            unregister_daemon(*this);
            quick_stop();
            delete module_registry_;
        }

        int Daemon::parse_cmdline(
            int argc, 
            char const * argv[])
        {
            char const ** p = argv;
            for (int i = 0; i < argc; ++i) {
                if (argv[i][0] == '-' && argv[i][1] == '-') {
                    config_.profile().pre_set(argv[i] + 2);
                } else if (argv[i][0] == '+' && argv[i][1] == '+') {
                    config_.profile().post_set(argv[i] + 2);
                } else {
                    *p++ = argv[i];
                }
            }
            return p - argv;
        }

        static void startup_notify(
            boost::system::error_code & result2, 
            boost::mutex & mutex, 
            boost::condition_variable & cond, 
            boost::system::error_code const & result)
        {
            boost::mutex::scoped_lock lock(mutex);
            result2 = result;
            cond.notify_all();
        }

        boost::system::error_code Daemon::start(
            size_t concurrency)
        {
            io_work_ = new boost::asio::io_service::work(io_svc_);
            boost::system::error_code result;
            LOG_S(Logger::kLevelDebug, "[start] beg");
            if (concurrency == 0) {
                result = module_registry_->startup();
            } else {
                boost::mutex mutex;
                boost::condition_variable cond;
                boost::mutex::scoped_lock lock(mutex);
                io_svc_.post(boost::bind(startup_notify, 
                    boost::ref(result), 
                    boost::ref(mutex), 
                    boost::ref(cond), 
                    boost::bind(&detail::ModuleRegistry::startup, module_registry_)));
                for (size_t i = 0; i < concurrency; ++i) {
                    th_grp_.create_thread(boost::bind(&boost::asio::io_service::run, &io_svc_));
                }
                cond.wait(lock);
            }
            LOG_S(Logger::kLevelDebug, "[start] end");
            if (result) {
                LOG_S(Logger::kLevelDebug, "[stop] beg");
                delete io_work_;
                io_work_ = NULL;
                run();
            }
            return result;
        }

        boost::system::error_code Daemon::resume()
        {
            boost::system::error_code result;
            boost::mutex mutex;
            boost::condition_variable cond;
            boost::mutex::scoped_lock lock(mutex);
            io_svc_.post(boost::bind(startup_notify, 
                boost::ref(result), 
                boost::ref(mutex), 
                boost::ref(cond), 
                boost::bind(&detail::ModuleRegistry::resume, module_registry_)));
            cond.wait(lock);
            return result;
        }

        boost::system::error_code Daemon::pause()
        {
            boost::system::error_code result;
            boost::mutex mutex;
            boost::condition_variable cond;
            boost::mutex::scoped_lock lock(mutex);
            io_svc_.post(boost::bind(startup_notify, 
                boost::ref(result), 
                boost::ref(mutex), 
                boost::ref(cond), 
                boost::bind(&detail::ModuleRegistry::pause, module_registry_)));
            cond.wait(lock);
            return result;
        }

        boost::system::error_code Daemon::start(
            start_call_back_type const & start_call_back)
        {
            LOG_S(Logger::kLevelDebug, "[start] beg");
            io_work_ = new boost::asio::io_service::work(io_svc_);
            boost::system::error_code result;
            result = module_registry_->startup();
            LOG_S(Logger::kLevelDebug, "[start] end");
            start_call_back(result);
            if (result) {
            	LOG_S(Logger::kLevelDebug, "[stop] beg");
                delete io_work_;
                io_work_ = NULL;
            }
            run();
            return result;
        }

        void Daemon::run()
        {
            if (th_grp_.size()) {
                th_grp_.join_all();
                io_svc_.reset();
            } else {
                io_svc_.run();
                io_svc_.reset();
            }
            LOG_S(Logger::kLevelDebug, "[stop] end");
        }

        void Daemon::stop(
            bool wait)
        {
            delete io_work_;
            io_work_ = NULL;
            io_svc_.post(
                boost::bind(&detail::ModuleRegistry::shutdown, module_registry_));
            if (wait) {
                LOG_S(Logger::kLevelDebug, "[stop] beg");
                run();
            }
        }

        void Daemon::post_stop()
        {
            io_svc_.post(boost::bind(&Daemon::stop, this, false));
        }

        void Daemon::quick_stop()
        {
            if (io_work_) {
                delete io_work_;
                io_work_ = NULL;
            }
            io_svc_.stop();
            if (th_grp_.size()) {
                th_grp_.join_all();
                io_svc_.reset();
            }
        }

        bool Daemon::is_started() const
        {
            return module_registry_->is_started();
        }

    } // namespace daemon
} // namespace util
