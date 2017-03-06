// Path.h

#ifndef _FRAMEWORK_FILE_SYSTEM_PATH_H_
#define _FRAMEWORK_FILE_SYSTEM_PATH_H_

#include <boost/filesystem/path.hpp>

namespace framework
{
    namespace filesystem
    {

        /// 获取当前可执行文件路径
        boost::filesystem::path bin_file();

        /// 获取临时文件存放的路径
        boost::filesystem::path temp_path();

        /// 获取framework临时文件的存放路径
        boost::filesystem::path framework_temp_path();

        /// 获取日志路径
        boost::filesystem::path log_path();

        /// 查找可执行文件所在的路径
        boost::filesystem::path which_bin_file(
            boost::filesystem::path const & file);

        /// 查找配置文件所在的路径
        boost::filesystem::path which_config_file(
            boost::filesystem::path const & file);

    } // namespace filesystem
} // namespace framework

#endif // _FRAMEWORK_FILE_SYSTEM_PATH_H_
