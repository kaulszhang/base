// ModuleRegistry.h

#ifndef _UTIL_DAEMON_DETAIL_MODULE_REGISTRY_H_
#define _UTIL_DAEMON_DETAIL_MODULE_REGISTRY_H_

#include "util/daemon/Daemon.h"
#include "util/daemon/detail/Module.h"

#include <boost/thread/mutex.hpp>

namespace util
{
    namespace daemon
    {
        namespace detail
        {

            struct no_arg {};

            template <
                typename Module
            >
            struct CreateModule
            {
                template <
                    typename Arg1, 
                    typename Arg2
                >
                static Module * apply(
                    util::daemon::Daemon & daemon, 
                    Arg1 arg1, 
                    Arg2 arg2)
                {
                    return new Module(daemon, arg1, arg2);
                }

                template <
                    typename Arg1
                >
                static Module * apply(
                    util::daemon::Daemon & daemon, 
                    Arg1 arg1, 
                    no_arg * arg2)
                {
                    return new Module(daemon, arg1);
                }

                static Module * apply(
                    util::daemon::Daemon & daemon, 
                    no_arg * arg1, 
                    no_arg * arg2)
                {
                    return new Module(daemon);
                }
            };

            class ModuleRegistry
            {
            public:
                ModuleRegistry(
                    util::daemon::Daemon & daemon);

                ~ModuleRegistry();

            public:
                boost::system::error_code startup();

                boost::system::error_code resume();

                boost::system::error_code pause();

                void shutdown();

                bool is_started() const
                {
                    return is_started_;
                }

            public:
                template <
                    typename Module, 
                    typename Arg1, 
                    typename Arg2
                >
                Module & use_module(
                    Arg1 arg1, 
                    Arg2 arg2)
                {
                    (void)static_cast<Module *>(static_cast<Module*>(0));
                    (void)static_cast<detail::Id const *>(&Module::id);

                    boost::mutex::scoped_lock lock(mutex_);

                    // First see if there is an existing Module object for the given type.
                    util::daemon::detail::Module * module = first_module_;
                    while (module) {
                        if (module_id_matches(*module, Module::id))
                            return *static_cast<Module *>(module);
                        module = module->next_;
                    }

                    // Create a new Module object. The Module registry's mutex is not locked
                    // at this time to allow for nested calls into this function from the new
                    // Module's constructor.
                    lock.unlock();
                    std::auto_ptr<Module> new_module(CreateModule<Module>::apply(daemon_, arg1, arg2));
                    init_module_id(*new_module, Module::id);
                    Module & new_module_ref = *new_module;
                    lock.lock();

                    // Check that nobody else created another Module object of the same type
                    // while the lock was released.
                    module = first_module_;
                    while (module) {
                        if (module_id_matches(*module, Module::id))
                            return *static_cast<Module*>(module);
                        module = module->next_;
                    }

                    // Module was successfully initialised, pass ownership to registry.
                    new_module->prev_ = last_module_;
                    if (last_module_) {
                        last_module_ = last_module_->next_ = new_module.release();
                    } else {
                        first_module_ = last_module_ = new_module.release();
                    }
                    if (is_started_)
                        static_cast<detail::Module &>(new_module_ref).startup();
                    return new_module_ref;
                }

                // Add a Module object. Returns false on error, in which case ownership of
                // the object is retained by the caller.
                template <typename Module>
                bool add_module(Module* new_module)
                {
                    (void)static_cast<Module *>(static_cast<Module*>(0));
                    (void)static_cast<detail::Id const *>(&Module::id);

                    boost::mutex::scoped_lock lock(mutex_);

                    // Check if there is an existing Module object for the given type.
                    util::daemon::detail::Module * module = first_module_;
                    while (module) {
                        if (module_id_matches(*module, Module::id))
                            return false;
                        module = module->next_;
                    }

                    // Take ownership of the Module object.
                    init_module_id(*new_module, Module::id);
                    new_module->prev_ = last_module_;
                    if (last_module_) {
                        last_module_ = last_module_->next_ = new_module;
                    } else {
                        first_module_ = last_module_ = new_module;
                    }

                    return true;
                }

                // Check whether a Module object of the specified type already exists.
                template <typename Module>
                bool has_module() const
                {
                    (void)static_cast<Module *>(static_cast<Module*>(0));
                    (void)static_cast<detail::Id const *>(&Module::id);

                    boost::mutex::scoped_lock lock(mutex_);

                    util::daemon::detail::Module * module = first_module_;
                    while (module) {
                        if (module_id_matches(*module, Module::id))
                            return true;
                        module = module->next_;
                    }

                    return false;
                }

            private:
                static void init_module_id(
                    util::daemon::detail::Module & module,
                    util::daemon::detail::Id const & id)
                {
                    module.id_ = &id;
                }

                static bool module_id_matches(
                    util::daemon::detail::Module const & module,
                    util::daemon::detail::Id const & id)
                {
                    return module.id_ == &id;
                }

            private:
                util::daemon::Daemon & daemon_;
                mutable boost::mutex mutex_;
                util::daemon::detail::Module * first_module_;
                util::daemon::detail::Module * last_module_;
                bool is_started_;
                bool is_paused_;
            };

        } // namespace detail
    } // namespace daemon
} // namespace util

#endif // _UTIL_DAEMON_DETAIL_MODULE_REGISTRY_H_
