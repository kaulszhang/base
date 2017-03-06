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
            // ���ò�����ʼ����������
            typedef void (*initor_type)(
                ConfigItem &, 
                std::string const &);

            // ���ò�����������������
            typedef boost::system::error_code (*settor_type)(
                ConfigItem &, 
                std::string const &);

            // ���ò�����ȡ����������
            typedef boost::system::error_code (*gettor_type)(
                ConfigItem const &, 
                std::string &);

            // ��������������
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

            initor_type initor_; // ��ʼ����
            settor_type settor_; // ������
            gettor_type gettor_; // ��ȡ��
            deletor_type deletor_; // ������
        };

        // Ĭ�ϵ�������
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
            // ֱ�ӽ����ַ�������ʼ������ֵ
            static void init(
                ConfigItem & item, 
                std::string const & str)
            {
                C & this_item = static_cast<C &>(item);
                T t(this_item.init_value());
                if (!framework::string::parse2(str, t))
                    this_item.init_value(t);
            }

            // ֱ�ӽ����ַ������޸Ĳ���ֵ����
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

            // ֱ�ӴӲ���ֵ�����ʽ������ַ���
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

            // ������
            void init_value(
                T const & t)
            {
            }

            // ������
            boost::system::error_code set_value(
                T const & t)
            {
                return framework::system::logic_error::no_permission;
            }

            // ������
            boost::system::error_code get_value(
                T & t) const
            {
                return framework::system::logic_error::no_permission;
            }
        };

        // ������ɱ�־����
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

            // ֱ���޸Ĳ���ֵ����
            void init_value(
                T const & t)
            {
                t_ = t;
            }

            // ֱ���޸Ĳ���ֵ����
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

            // ֱ�ӴӲ���ֵ�����ʽ������ַ���
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
