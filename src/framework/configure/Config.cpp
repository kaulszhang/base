// Config.cpp

#include "framework/Framework.h"
#include "framework/configure/Config.h"
#include "framework/filesystem/Path.h"

namespace framework
{
    namespace configure
    {

        using boost::system::error_code;
        using framework::system::logic_error::succeed;
        using framework::system::logic_error::item_not_exist;
        using framework::system::logic_error::no_permission;

        Config::Config()
        {
            boost::filesystem::path ph = framework::filesystem::bin_file();
            ph.replace_extension(".ini");
            pf_.load(ph.string());
        }

        Config::Config(
            std::string const & file)
            : pf_(framework::filesystem::which_config_file(file).string())
        {
        }

        Config::~Config(void)
        {
        }

        ConfigModule & Config::register_module(
            std::string const & module)
        {
            iterator im = insert(std::make_pair(module, ConfigModule(module, *this))).first;
            return im->second;
        }

        void Config::register_param(
            std::string const & module, 
            std::string const & key, 
            ConfigItem * item) const
        {
            std::string value;
            if (pf_.get(module, key, value) == 0) {
                item->init(value);
            }
        }

        error_code Config::set(
            std::string const & m, 
            std::string const & k, 
            std::string const & v, 
            bool save)
        {
            iterator im = find(m);
            if (im == end())
                return error_code(item_not_exist);
            error_code ec = im->second.set(k, v);
            if (!ec && save)
                pf_.set(m, k, v);
            return ec;
        }

        error_code Config::set_force(
            std::string const & m, 
            std::string const & k, 
            std::string const & v)
        {
            error_code ec = set(m, k, v, true);
            if (ec == no_permission || ec == item_not_exist) {
                // 即使不存在，或者没权限，也让设置
                pf_.set(m, k, v);
                //ec = success;
            }
            return ec;
        }

        error_code Config::get(
            std::string const & m, 
            std::string const & k, 
            std::string & v)
        {
            const_iterator im = find(m);
            if (im == end())
                return item_not_exist;
            return im->second.get(k, v);
        }

        error_code Config::get_force(
            std::string const & m, 
            std::string const & k, 
            std::string & v)
        {
            error_code ec = get(m, k, v);
            if (ec == no_permission || ec == item_not_exist) {
                pf_.get(m, k, v);
            }
            return ec;
        }

        error_code Config::get(
            std::string const & m, 
            std::map<std::string, std::string> & kvs)
        {
            const_iterator im = find(m);
            if (im == end())
                return item_not_exist;
            return im->second.get(kvs);
        }

        error_code Config::get(
            std::map<std::string, std::map<std::string, std::string> > & mkvs)
        {
            const_iterator im = begin();
            for (; im != end(); ++im) {
                std::map<std::string, std::string> & kvs = mkvs[im->first];
                im->second.get(kvs);
            }
            return succeed;
        }

        error_code Config::sync(
            std::string const & m, 
            std::string const & k)
        {
            std::string v;
            error_code ec = get(m, k, v);
            if (ec)
                return ec;
            pf_.set(m, k, v);
            return succeed;
        }

        boost::system::error_code Config::sync(
            std::string const & m)
        {
            const_iterator im = find(m);
            std::map<std::string, std::string> kvs;
            error_code ec = get(m, kvs);
            if (ec)
                return ec;
            for (std::map<std::string, std::string>::const_iterator iter = kvs.begin(); iter != kvs.end(); ++iter) {
                pf_.set(m, iter->first, iter->second, false);
            }
            pf_.save();
            return succeed;
        }

        boost::system::error_code Config::sync()
        {
            const_iterator im = begin();
            for (; im != end(); ++im) {
                std::map<std::string, std::string> kvs;
                im->second.get(kvs);
                for (std::map<std::string, std::string>::const_iterator iter = kvs.begin(); iter != kvs.end(); ++iter) {
                    pf_.set(im->first, iter->first, iter->second, false);
                }
            }
            pf_.save();
            return succeed;
        }

    } // namespace configure
} // namespace framework
