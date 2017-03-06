// Path.h

#ifndef _FRAMEWORK_FILE_SYSTEM_PATH_H_
#define _FRAMEWORK_FILE_SYSTEM_PATH_H_

#include <boost/filesystem/path.hpp>

namespace framework
{
    namespace filesystem
    {

        /// ��ȡ��ǰ��ִ���ļ�·��
        boost::filesystem::path bin_file();

        /// ��ȡ��ʱ�ļ���ŵ�·��
        boost::filesystem::path temp_path();

        /// ��ȡframework��ʱ�ļ��Ĵ��·��
        boost::filesystem::path framework_temp_path();

        /// ��ȡ��־·��
        boost::filesystem::path log_path();

        /// ���ҿ�ִ���ļ����ڵ�·��
        boost::filesystem::path which_bin_file(
            boost::filesystem::path const & file);

        /// ���������ļ����ڵ�·��
        boost::filesystem::path which_config_file(
            boost::filesystem::path const & file);

    } // namespace filesystem
} // namespace framework

#endif // _FRAMEWORK_FILE_SYSTEM_PATH_H_
