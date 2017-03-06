// Module.h

#ifndef _UTIL_DAEMON_MODULE_H_
#define _UTIL_DAEMON_MODULE_H_

#include "util/daemon/Daemon.h"
#include "util/daemon/detail/Module.h"

namespace util
{
    namespace daemon
    {

        template <typename Type>
        class ModuleBase
            : public detail::Module
        {
        public:
            static detail::ModuleId<Type> id;

            // Constructor.
            ModuleBase(
                Daemon & daemon, 
                std::string const & name = "")
                : detail::Module(daemon, name)
            {
            }
        };

        template <typename Type>
        detail::ModuleId<Type> ModuleBase<Type>::id;

    } // namespace daemon
} // namespace util

#endif // _UTIL_DAEMON_MODULE_H_
