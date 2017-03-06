// Daemon.h

#ifndef _UTIL_DAEMON_DAEMON_H_
#define _UTIL_DAEMON_DAEMON_H_

#include <framework/configure/Config.h>
#include <framework/logger/Logger.h>

#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>
#include <boost/function.hpp>

namespace util
{
    namespace daemon
    {

        namespace detail
        {
            class ModuleRegistry;
        }

        class Daemon;

        template <typename Module>
        Module & use_module(
            Daemon & daemon);

        template <typename Module>
        void add_module(
            Daemon & daemon, 
            Module * module);

        template <typename Module>
        bool has_module(
            Daemon & daemon);

        class Daemon
        {
        public:
            Daemon();

            Daemon(
                std::string const & conf);

            ~Daemon();

        public:
            int parse_cmdline(
                int argc, 
                char const * argv[]);

            /* 
             * concurrency == 0 不执行io_service::run，需要外部再调用run（同一线程），外面可以在io_service::run线程内部调用stop(false)停止，在io_service::run线程外部需要调用post_stop()停止
             * concurrency != 0 创建相应数目的线程，来执行io_service::run，外面可以调用stop(true)停止，等待io_service结束
             * start 完成后，函数返回
             * start 如果失败，会等到行io_service结束，不需要再执行stop
             */
            boost::system::error_code start(
                size_t concurrency = 0);

            void run();

            typedef boost::function<
                void (boost::system::error_code const &)
            > start_call_back_type;

            /* 
             * start 完成后，执行callback，然后继续等待io_service结束
             * 外面可以在线程内部调用stop(false)停止，在线程外部需要调用post_stop()停止
             * start 如果失败，会等到行io_service结束，不需要再执行stop
             */
            boost::system::error_code start(
                start_call_back_type const & start_call_back);

            boost::system::error_code resume();

            boost::system::error_code pause();

            void stop(
                bool wait = true);


            void post_stop();

            void quick_stop();

        public:
            bool is_started() const;

        public:
            static Daemon & from_io_svc(
                boost::asio::io_service & io_svc);

        public:
            boost::asio::io_service & io_svc()
            {
                return io_svc_;
            }

            framework::configure::Config & config()
            {
                return config_;
            }

            framework::logger::Logger & logger()
            {
                return logger_;
            }

        public:
            template <
                typename Module
            >
            friend Module & use_module(
                Daemon & daemon);

            template <
                typename Module, 
                typename Arg1
            >
            friend Module & use_module(
                Daemon & daemon, 
                Arg1 arg1);

            template <
                typename Module, 
                typename Arg1, 
                typename Arg2
            >
            friend Module & use_module(
                Daemon & daemon, 
                Arg1 arg1, 
                Arg2 arg2);

            template <typename Module>
            friend void add_module(
                Daemon & daemon, 
                Module * module);

            template <typename Module>
            friend bool has_module(
                Daemon & daemon);

        private:
            static void register_daemon(
                Daemon & daemon);

            static void unregister_daemon(
                Daemon & daemon);

        private:
            boost::asio::io_service io_svc_;
            boost::asio::io_service::work * io_work_;
            boost::thread_group th_grp_;
            framework::configure::Config config_;
            framework::logger::Logger logger_;

            detail::ModuleRegistry * module_registry_;
        };

    } // namespace daemon
} // namespace util

#include "util/daemon/detail/ModuleRegistry.h"

namespace util
{
    namespace daemon
    {

        template <
            typename Module
        >
        Module & use_module(
            Daemon & daemon)
        {
            return daemon.module_registry_->use_module<Module>((detail::no_arg *)NULL, (detail::no_arg *)NULL);
        }

        template <
            typename Module, 
            typename Arg1
        >
        Module & use_module(
            Daemon & daemon, 
            Arg1 arg1)
        {
            return daemon.module_registry_->use_module<Module>(arg1, (detail::no_arg *)NULL);
        }

        template <
            typename Module, 
            typename Arg1, 
            typename Arg2
        >
        Module & use_module(
            Daemon & daemon, 
            Arg1 arg1, 
            Arg2 arg2)
        {
            return daemon.module_registry_->use_module<Module>(arg1, arg2);
        }

        template <typename Module>
        void add_module(
            Daemon & daemon, 
            Module * module)
        {
            assert(&daemon == &module->get_daemon());

            daemon.module_registry_->add_module<Module>(module);
        }

        template <typename Module>
        bool has_module(
            Daemon & daemon)
        {
            return daemon.module_registry_->has_module<Module>();
        }

        template <typename Module>
        Module & use_module(
            boost::asio::io_service & io_svc)
        {
            return use_module<Module>(Daemon::from_io_svc(io_svc));
        }

        template <typename Module>
        void add_module(
            boost::asio::io_service & io_svc, 
            Module * module)
        {
            add_module<Module>(Daemon::from_io_svc(io_svc), module);
        }

        template <typename Module>
        bool has_module(
            boost::asio::io_service & io_svc)
        {
            return has_module<Module>(Daemon::from_io_svc(io_svc));
        }

    } // namespace daemon
} // namespace util

#endif // _UTIL_DAEMON_DAEMON_H_
