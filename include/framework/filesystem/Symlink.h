// Symlink.h

#ifndef _FRAMEWORK_FILE_SYSTEM_SYMLINK_H_
#define _FRAMEWORK_FILE_SYSTEM_SYMLINK_H_

#include <boost/filesystem/path.hpp>

namespace framework
{
    namespace filesystem
    {

        boost::filesystem::path read_symlink(
            boost::filesystem::path const & ph, 
            boost::system::error_code & ec);

    } // namespace filesystem
} // namespace framework

#endif // _FRAMEWORK_FILE_SYSTEM_SYMLINK_H_
