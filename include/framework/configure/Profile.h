// Profile.h

/** 一个配置文件读写类
配置文件内容采用一下格式：
[模块1]
配置参数1=具体值
配置参数2=具体值
[模块2]
配置参数1=具体值
*/

#ifndef _FRAMEWORK_CONFIGURE_PROFILE_H_
#define _FRAMEWORK_CONFIGURE_PROFILE_H_

namespace framework
{
    namespace configure
    {

        class Profile
        {
        public:
            // 构造一个空的配置集
            Profile();

            // 从配置文件读取配置集
            Profile(
                std::string const & file);

            ~Profile();

        public:
            // 从配置文件读取配置集
            int load(
                std::string const & file);

            // 向配置文件写入配置集
            int save();

            // 向配置文件写入配置集
            int save(
                std::string const & file);

        public:
            // 获取一个字符串类型配置参数的值
            int get(
                std::string const & section, 
                std::string const & key, 
                std::string & val) const;

            // 设置一个字符串类型配置参数的值
            int set(
                std::string const & section, 
                std::string const & key, 
                std::string const & val, 
                bool save = true);

            // 设置一个字符串类型配置参数的值，参数形如“模块1.配置参数1=具体值”
            int pre_set(
                std::string const & line);

            int post_set(
                std::string const & line);

            void get_all(
                std::map<std::string, std::map<std::string, std::string> > & mkvs);

        private:
            std::string file_; // 保存配置信息的文件名
            std::map<std::string, std::map<std::string, std::string> > sec_key_vals; // 配置参数集
            std::map<std::string, std::map<std::string, std::string> > pre_sec_key_vals; // 配置参数集
            std::map<std::string, std::map<std::string, std::string> > post_sec_key_vals; // 配置参数集
        };

    } // namespace configure
} // namespace framework

#endif // _FRAMEWORK_CONFIGURE_PROFILE_H_
