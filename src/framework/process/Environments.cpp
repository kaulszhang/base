// Environments.cpp

#include "framework/Framework.h"
#include "framework/process/Environments.h"

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#else
#  include <stdlib.h>
#endif

namespace framework
{
    namespace process
    {

        std::string get_environment(
            std::string const & key, 
            std::string const & def)
        {
#ifndef BOOST_WINDOWS_API
            char * value;
            if ((value = ::getenv(key.c_str()))) {
                return value;
            } else {
                return def;
            }
#else
            char Buffer[64];
            if (::GetEnvironmentVariable(
                "FRAMEWORK_PROCESS_PARENT_ID", 
                Buffer, 
                sizeof(Buffer)) > 0) {
                    return Buffer;
            } else {
                return def;
            }
#endif
        }

        void set_environment(
            std::string const & key, 
            std::string const & value)
        {
#ifndef BOOST_WINDOWS_API
            ::setenv(key.c_str(), value.c_str(), 1 /* overwrite */);
#else
            ::SetEnvironmentVariable(
                key.c_str(), 
                value.c_str());
#endif
        }

        void clear_environment(
            std::string const & key)
        {
#ifndef BOOST_WINDOWS_API
            ::unsetenv(key.c_str());
#else
            ::SetEnvironmentVariable(
                key.c_str(), 
                NULL);
#endif
        }

    } // namespace process
} // namespace framework
