// Config.h

#ifndef __FRAMEWORK_CONFIGURE_CONFIG_ITEM_H_
#define __FRAMEWORK_CONFIGURE_CONFIG_ITEM_H_

#include "framework/string/Format.h"
#include "framework/string/Parse.h"
#include "framework/system/LogicError.h"

//#include <boost/function.hpp>

namespace framework
{
    namespace configure
    {
        class Config;

        class ConfigItem
        {
        public:
            // 配置参数初始化函数类型
            typedef void (*initor_type)(
                ConfigItem &, 
                std::string const &);

            // 配置参数设置器函数类型
            typedef boost::system::error_code (*settor_type)(
                ConfigItem &, 
                std::string const &);

            // 配置参数读取器函数类型
            typedef boost::system::error_code (*gettor_type)(
                ConfigItem const &, 
                std::string &);

            // 销毁器函数类型
            typedef void (*deletor_type)(
                ConfigItem &);

        protected:
            ConfigItem()
                : initor_(NULL)
                , settor_(NULL)
                , gettor_(NULL)
                , deletor_(NULL)
            {
            }

            ConfigItem(
                unsigned int flag, 
                initor_type initor, 
                settor_type settor, 
                gettor_type gettor, 
                deletor_type deletor)
                : initor_(initor)
                , settor_(settor)
                , gettor_(gettor)
                , deletor_(deletor)
            {
            }

        private:
            // non copyable
            ConfigItem(
                ConfigItem const & r);

            ConfigItem & operator=(
                ConfigItem const & r);

        public:
            void init(
                std::string const & str)
            {
                initor_(*this, str);
            }

            boost::system::error_code set(
                std::string const & str)
            {
                return settor_(*this, str);
            }

            boost::system::error_code get(
                std::string & str) const
            {
                return gettor_(*this, str);
            }

            void del()
            {
                deletor_(*this);
            }

            initor_type initor_; // 初始化器
            settor_type settor_; // 设置器
            gettor_type gettor_; // 读取器
            deletor_type deletor_; // 销毁器
        };

        // 默认的配置器
        template <typename T, typename C>
        struct ConfigItemHelper
            : public ConfigItem
        {
            ConfigItemHelper( unsigned int flag = 0)
                : ConfigItem(
                    flag,
                    C::init,
                    C::set, 
                    C::get, 
                    C::del)
            {
            }

        private:
            // 直接解析字符串并初始化参数值
            static void init(
                ConfigItem & item, 
                std::string const & str)
            {
                C & this_item = static_cast<C &>(item);
                T t(this_item.init_value());
                if (!framework::string::parse2(str, t))
                    this_item.init_value(t);
            }

            // 直接解析字符串并修改参数值本身
            static boost::system::error_code set(
                ConfigItem & item, 
                std::string const & str)
            {
                C & this_item = static_cast<C &>(item);
                T t(this_item.init_value());
                boost::system::error_code ec = framework::string::parse2(str, t);
                if (!ec)
                    ec = this_item.set_value(t);
                return ec;
            }

            // 直接从参数值本身格式化结果字符串
            static boost::system::error_code get(
                ConfigItem const & item, 
                std::string & str)
            {
                C const & this_item = static_cast<C const &>(item);
                T t = T();
                boost::system::error_code ec = this_item.get_value(t);
                if (!ec)
                    ec = framework::string::format2(str, t);
                return ec;
            }

            static void del(
                ConfigItem & item)
            {
                C & this_item = static_cast<C &>(item);
                delete &this_item;
            }

            T init_value() const
            {
                return T();
            }

            // 不允许
            void init_value(
                T const & t)
            {
            }

            // 不允许
            boost::system::error_code set_value(
                T const & t)
            {
                return framework::system::logic_error::no_permission;
            }

            // 不允许
            boost::system::error_code get_value(
                T & t) const
            {
                return framework::system::logic_error::no_permission;
            }
        };

        // 访问许可标志定义
        enum Flag {
            allow_set = 1, 
            allow_get = 2, 
        };

        template <typename T>
        struct ConfigItemT
            : public ConfigItemHelper<T, ConfigItemT< T > >
        {
        public:
            ConfigItemT(
                T & t, 
                unsigned int flag)
                : ConfigItemHelper<T, ConfigItemT>( flag )
                , t_(t)
                , flag_(flag)
            {
            }

        public:
            T init_value() const
            {
                return t_;
            }

            // 直接修改参数值本身
            void init_value(
                T const & t)
            {
                t_ = t;
            }

            // 直接修改参数值本身
            boost::system::error_code set_value(
                T const & t)
            {
                if (flag_ & allow_set) {
                    t_ = t;
                    return boost::system::error_code();
                } else {
                    return framework::system::logic_error::no_permission;
                }
            }

            // 直接从参数值本身格式化结果字符串
            boost::system::error_code get_value(
                T & t) const
            {
                if (flag_ & allow_get) {
                    t =  t_;
                    return boost::system::error_code();
                } else {
                    return framework::system::logic_error::no_permission;
                }
            }

        private:
            T & t_;
            unsigned int flag_;
        };

        template <typename T>
        static ConfigItemT<T> * make_item(
            T & t, 
            unsigned int flag)
        {
            return new ConfigItemT<T>(t, flag);
        }

    } // namespace configure
} // namespace framework

#define CONFIG_PARAM(p, flag) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (flag)) \
    )

#define CONFIG_PARAM_NOACC(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (0)) \
    )

#define CONFIG_PARAM_RDONLY(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (framework::configure::allow_get)) \
    )

#define CONFIG_PARAM_WRONLY(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (framework::configure::allow_set)) \
    )

#define CONFIG_PARAM_RDWR(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, \
    (framework::configure::allow_get | framework::configure::allow_set)) \
    )

#define CONFIG_PARAM_NAME(name, p, flag) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (flag)) \
    )

#define CONFIG_PARAM_NAME_NOACC(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (0)) \
    )

#define CONFIG_PARAM_NAME_RDONLY(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (framework::configure::allow_get)) \
    )

#define CONFIG_PARAM_NAME_WRONLY(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (framework::configure::allow_set)) \
    )

#define CONFIG_PARAM_NAME_RDWR(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, \
    (framework::configure::allow_get | framework::configure::allow_set)) \
    )


#endif // __FRAMEWORK_CONFIGURE_CONFIG_ITEM_H_
