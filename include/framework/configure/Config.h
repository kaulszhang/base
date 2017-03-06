// Config.h

/** Config用于读写配置参数，以及动态获取和重新配置参数值
* 支持多种数据类型，只要Parse模板支持该数据类型
* 配置参数按模块分组，不同组的参数名字可以重复使用
* 支持访问控制，分别设置读写许可
* 可以指定自定义的读取器和设置器，用自定义的方式响应读写请求
*/

#ifndef __FRAMEWORK_CONFIGURE_CONFIG_H_
#define __FRAMEWORK_CONFIGURE_CONFIG_H_

#include "framework/configure/Profile.h"
#include "framework/configure/ConfigItem.h"

namespace framework
{
    namespace configure
    {
        class Config;

        class ConfigModule
            : private std::map<std::string, ConfigItem *>
        {
        public:
            ConfigModule(
                std::string const & name, 
                Config & conf)
                : name_(name)
                , conf_(conf)
            {
            }

            ~ConfigModule()
            {
                for (iterator ik = begin(); ik != end(); ++ik) {
                    (ik->second)->del();
                    (ik->second) = NULL;
                }
            }

        public:
            ConfigModule & operator()(
                std::string const & key, 
                ConfigItem * item);

            template <typename T>
            ConfigModule & operator()(
                std::string const & key, 
                T & t, 
                unsigned int flag)
            {
                (*this)(key, make_item(flag, t));
                return *this;
            }

            ConfigModule & operator()(
                std::pair<std::string const, ConfigItem *> key_item)
            {
                (*this)(key_item.first, key_item.second);
                return *this;
            }

            ConfigModule & operator<<(
                std::pair<std::string const, ConfigItem *> key_item)
            {
                (*this)(key_item.first, key_item.second);
                return *this;
            }

            boost::system::error_code set(
                std::string const & key, 
                std::string const & value)
            {
                const_iterator ik = find(key);
                if (ik == end())
                    return framework::system::logic_error::item_not_exist;
                return ik->second->set(value);
            }

            boost::system::error_code get(
                std::string const & key, 
                std::string & value) const
            {
                const_iterator ik = find(key);
                if (ik == end())
                    return framework::system::logic_error::item_not_exist;
                return ik->second->get(value);
            }

            boost::system::error_code get(
                std::map<std::string, std::string> & kvs) const
            {
                for (const_iterator ik = begin(); ik != end(); ++ik) {
                    std::string value;
                    if (!ik->second->get(value)) {
                        kvs[ik->first] = value;
                    }
                }
                return framework::system::logic_error::succeed;
            }

        private:
            std::string name_;
            Config const & conf_;
        };

        class Config
            : private std::map<std::string, ConfigModule>
        {
        public:
            Config();

            Config(
                std::string const & file);

            ~Config(void);

        public:
            // 返回只读的配置文件内存镜像
            Profile & profile()
            {
                return pf_;
            }

        public:
            // 注册一组配置参数，它们属于同一个模块
            ConfigModule & register_module(
                std::string const & module);

            // 注册一个配置参数
            template <typename T>
            void register_param(
                std::string const & module, 
                std::string const & key, 
                T & t,
                unsigned int flag)
            {
                register_param(module, key, make_item(t, flag));
            }

            // 设置配置参数值
            boost::system::error_code set(
                std::string const & m, 
                std::string const & k, 
                std::string const & v, 
                bool save = false);

            boost::system::error_code set_force(
                std::string const & m, 
                std::string const & k, 
                std::string const & v);

            // 读取配置参数值
            boost::system::error_code get(
                std::string const & m, 
                std::string const & k, 
                std::string & v);

            // 读取配置参数值
            boost::system::error_code get_force(
                std::string const & m, 
                std::string const & k, 
                std::string & v);

            // 获取一个模块的所有配置参数的名称及数值
            boost::system::error_code get(
                std::string const & m, 
                std::map<std::string, std::string> & kvs);

            // 获取所有配置参数的名称及数值
            boost::system::error_code get(
                std::map<std::string, std::map<std::string, std::string> > & mkvs);

            boost::system::error_code sync(
                std::string const & m, 
                std::string const & k);

            boost::system::error_code sync(
                std::string const & m);

            boost::system::error_code sync();

        private:
            friend class ConfigModule;

            // 注册一个配置参数
            void register_param(
                std::string const & module, 
                std::string const & key, 
                ConfigItem * item) const;

        private:
            Profile pf_; // 配置文件内存镜像
        };

        inline ConfigModule & ConfigModule::operator()(
            std::string const & key, 
            ConfigItem * item)
        {
            iterator it = find( key );
            if ( it != end() )
                (it->second)->del();

            (*this)[key] = item;

            conf_.register_param(name_, key, item);
            return *this;
        }

    } // namespace configure
} // namespace framework

#endif // __FRAMEWORK_CONFIGURE_CONFIG_H_
