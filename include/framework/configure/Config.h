// Config.h

/** Config���ڶ�д���ò������Լ���̬��ȡ���������ò���ֵ
* ֧�ֶ����������ͣ�ֻҪParseģ��֧�ָ���������
* ���ò�����ģ����飬��ͬ��Ĳ������ֿ����ظ�ʹ��
* ֧�ַ��ʿ��ƣ��ֱ����ö�д���
* ����ָ���Զ���Ķ�ȡ���������������Զ���ķ�ʽ��Ӧ��д����
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
            // ����ֻ���������ļ��ڴ澵��
            Profile & profile()
            {
                return pf_;
            }

        public:
            // ע��һ�����ò�������������ͬһ��ģ��
            ConfigModule & register_module(
                std::string const & module);

            // ע��һ�����ò���
            template <typename T>
            void register_param(
                std::string const & module, 
                std::string const & key, 
                T & t,
                unsigned int flag)
            {
                register_param(module, key, make_item(t, flag));
            }

            // �������ò���ֵ
            boost::system::error_code set(
                std::string const & m, 
                std::string const & k, 
                std::string const & v, 
                bool save = false);

            boost::system::error_code set_force(
                std::string const & m, 
                std::string const & k, 
                std::string const & v);

            // ��ȡ���ò���ֵ
            boost::system::error_code get(
                std::string const & m, 
                std::string const & k, 
                std::string & v);

            // ��ȡ���ò���ֵ
            boost::system::error_code get_force(
                std::string const & m, 
                std::string const & k, 
                std::string & v);

            // ��ȡһ��ģ����������ò��������Ƽ���ֵ
            boost::system::error_code get(
                std::string const & m, 
                std::map<std::string, std::string> & kvs);

            // ��ȡ�������ò��������Ƽ���ֵ
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

            // ע��һ�����ò���
            void register_param(
                std::string const & module, 
                std::string const & key, 
                ConfigItem * item) const;

        private:
            Profile pf_; // �����ļ��ڴ澵��
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
