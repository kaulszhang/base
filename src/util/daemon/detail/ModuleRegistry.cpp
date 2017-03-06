// ModuleRegistry.cpp

#include "util/Util.h"
#include "util/daemon/Daemon.h"

#include <framework/logger/LoggerStreamRecord.h>
using framework::logger::Logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Daemon", 0);

namespace util
{
    namespace daemon
    {
        namespace detail
        {

            ModuleRegistry::ModuleRegistry(
                Daemon & daemon)
                : daemon_(daemon)
                , first_module_(NULL)
                , last_module_(NULL)
                , is_started_(false)
                , is_paused_(false)
            {
            }

            ModuleRegistry::~ModuleRegistry()
            {
                while (last_module_) {
                    Module * prev_module = last_module_->prev_;
                    delete last_module_;
                    last_module_ = prev_module;
                }
            }

            boost::system::error_code ModuleRegistry::startup()
            {
                boost::mutex::scoped_lock lock(mutex_);
                boost::system::error_code ec;
                Module * module = first_module_;
                while (module) {
                    LOG_S(Logger::kLevelEvent, "starting module " << module->name());
                    ec = module->startup();
                    if (ec) {
                        LOG_S(Logger::kLevelAlarm, "start module " << module->name() << " failed: " << ec.message());
                        break;
                    }
                    module = module->next_;
                }
                if (module) {
                    module = module->prev_;
                    while (module) {
                        LOG_S(Logger::kLevelEvent, "shutdowning module " << module->name());
                        module->shutdown();
                        module = module->prev_;
                    }
                } else {
                    is_started_ = true;
                }
                return ec;
            }

            void ModuleRegistry::shutdown()
            {
                boost::mutex::scoped_lock lock(mutex_);
                if (!is_started_)
                    return;
                Module * module = last_module_;
                while (module) {
                    LOG_S(Logger::kLevelEvent, "shutdowning module " << module->name());
                    module->shutdown();
                    module = module->prev_;
                }
                is_started_ = false;
            }

            boost::system::error_code ModuleRegistry::resume()
            {
                boost::mutex::scoped_lock lock(mutex_);
                boost::system::error_code ec;
                
                if (!is_started_ || !is_paused_)
                {
                    LOG_S(Logger::kLevelError, "resume module failed. reason:" << ((!is_started_)?"server not started":"not paused before"));
                    return ec;
                }
                
                Module * module = first_module_;
                while (module) {
                    LOG_S(Logger::kLevelEvent, "resume module " << module->name());
                    ec = module->resume();
                    if (ec) {
                        LOG_S(Logger::kLevelAlarm, "resume module " << module->name() << " failed: " << ec.message());
                        break;
                    }
                    module = module->next_;
                }
                if (module) {
                    module = module->prev_;
                    while (module) {
                        LOG_S(Logger::kLevelEvent, "pause module " << module->name());
                        module->pause();
                        module = module->prev_;
                    }
                } else {
                    is_paused_ = false;
                }
                return ec;
            }

            boost::system::error_code ModuleRegistry::pause()
            {
                boost::mutex::scoped_lock lock(mutex_);
                boost::system::error_code ec;

                if (!is_started_ || is_paused_) {
                    return ec;
                }

                Module * module = last_module_;
                while (module) {
                    LOG_S(Logger::kLevelEvent, "pause module " << module->name());
                    module->pause();
                    module = module->prev_;
                }
                is_paused_ = true;
                return ec;
            }

        } // namespace detail
    } // namespace daemon
} // namespace util
