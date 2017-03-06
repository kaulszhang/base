// ModuleId.h

#ifndef _UTIL_DAEMON_DETAIL_MODULE_ID_H_
#define _UTIL_DAEMON_DETAIL_MODULE_ID_H_

#include "util/daemon/Daemon.h"

namespace util
{
    namespace daemon
    {
        namespace detail
        {

            class Id
            {
            public:
                /// Constructor.
                Id() {}
            };

            template <typename Type>
            class ModuleId
                : public Id
            {
            };

        } // namespace detail
    } // namespace daemon
} // namespace util

#endif // _UTIL_DAEMON_DETAIL_MODULE_ID_H_
