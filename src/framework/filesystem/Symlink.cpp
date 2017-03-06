// Path.cpp

#include "framework/Framework.h"
#include "framework/filesystem/Symlink.h"
#include "framework/system/ErrorCode.h"

using boost::filesystem::path;
using boost::system::error_code;

#ifdef BOOST_WINDOWS_API
#  include <windows.h>
#else
#  include <unistd.h>
#  define MAX_PATH 256
#endif

namespace framework
{
    namespace filesystem
    {

        path read_symlink(
            path const & ph, 
            error_code & ec)
        {
#ifndef BOOST_WINDOWS_API
            std::string ph2;
            ph2.resize(MAX_PATH);
            int ret = ::readlink(ph.string().c_str(), &ph2[0], MAX_PATH);
            if (ret > 0) {
                ec = error_code();
                ph2.resize(ret);
                return path(ph2);
            } else {
                ec = framework::system::last_system_error();
                return path();
            }
#else
            return path();
#endif
        }

    } // namespace filesystem
} // namespace framework
