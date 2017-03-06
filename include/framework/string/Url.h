// Url.h

#ifndef _FRAMEWORK_STRING_URL_H_
#define _FRAMEWORK_STRING_URL_H_

#include <framework/system/LogicError.h>
#include <framework/string/Algorithm.h>

namespace framework
{
    namespace string
    {

        class Url
        {
        public:
            static inline std::string truncate(
                std::string const & url, 
                size_t buffer_size)
            {
                return framework::string::truncate(url, buffer_size);
            }

            template<typename ByteType>
            static inline void truncate_to(
                std::string const & url, ByteType * buffer, size_t buffer_size)
            {
                return framework::string::truncate_to(url, buffer, buffer_size);
            }

            template<typename ByteType, size_t Size>
            static inline void truncate_to(
                std::string const & url, 
                ByteType(& buffer)[Size])
            {
                return framework::string::truncate_to(url, buffer);
           }

        public:
            static std::string encode(
                std::string const & str, 
                char const * ignore = "");

            static std::string decode(
                std::string const & str);

            static std::string encode_component(
                std::string const & str)
            {
                return encode(str, "!*()");
            }

            static std::string decode_component(
                std::string const & str)
            {
                return decode(str);
            }

        public:
            Url()
                : is_valid_(false)
            {
            }

            explicit Url(
                std::string const & url);

        public:
            std::string to_string() const;

            boost::system::error_code from_string(
                std::string const & url);

        public:
            void encode();

            void decode();

        public:
            bool is_valid() const
            {
                return is_valid_;
            }

            std::string const & protocol() const
            {
                return protocol_;
            }

            std::string const & user() const
            {
                return user_;
            }

            std::string const & password() const
            {
                return password_;
            }

            std::string const & host() const
            {
                return host_;
            }

            std::string const & svc() const
            {
                return svc_;
            }

            std::string const & svc_or(
                std::string const & def) const
            {
                return svc_.empty() ? def : svc_;
            }

            std::string host_svc() const
            {
                std::string hs = host_;
                if (!svc_.empty()) {
                    hs.append(1, ':');
                    hs.append(svc_);
                }
                return hs;
            }

            std::string const & path() const
            {
                return path_;
            }

            std::string path_all() const;

            std::string param(
                std::string const & key) const;

            std::string param_or(
                std::string const & key, 
                std::string const & def = "") const;

            std::string const & anchor() const
            {
                return anchor_;
            }

        public:
            friend bool operator==(
                Url const & l, 
                Url const & r)
            {
                return l.protocol_ == r.protocol_
                    && l.user_ == r.user_
                    && l.password_ == r.password_
                    && l.host_ == r.host_
                    && l.svc_ == r.svc_
                    && l.path_ == r.path_
                    && l.params_ == r.params_
                    && l.anchor_ == r.anchor_;
            }

        public:
            void protocol(
                std::string const & v)
            {
                protocol_ = v;
                is_valid_ = !protocol_.empty();
            }

            void user(
                std::string const & v)
            {
                user_ = v;
            }

            void password(
                std::string const & v)
            {
                password_ = v;
            }

            void host(
                std::string const & v)
            {
                host_ = v;
            }

            void svc(
                std::string const & v)
            {
                svc_ = v;
            }

            void path(
                std::string const & v)
            {
                path_ = v;
            }

            void param(
                std::string const & k, 
                std::string const & v);

            void anchor(
                std::string const & v)
            {
                anchor_ = v;
            }

            class Parameter
            {
            public:
                Parameter()
                {
                    line_ = "=";
                    pos_eq_ = 0;
                }

                Parameter(
                    std::string const & key)
                {
                    pos_eq_ = key.size();
                    line_ = key + "=";
                }

                Parameter(
                    std::string const & key, 
                    std::string const & value)
                {
                    pos_eq_ = key.size();
                    line_ = key + "=" + value;
                }

                Parameter & operator=(
                    std::string const & value)
                {
                    line_.replace(pos_eq_ + 1, line_.size() - pos_eq_ - 1, value);
                    return *this;
                }

                friend bool operator==(
                    Parameter const & l, 
                    Parameter const & r)
                {
                    return l.line_ == r.line_;
                }

                friend bool operator==(
                    Parameter const & l, 
                    std::string const & key)
                {
                    return l.pos_eq_ == key.size() 
                        && l.line_.compare(0, l.pos_eq_, key) == 0;
                }

                std::string key() const
                {
                    return line_.substr(0, pos_eq_);
                }

                operator std::string () const
                {
                    return key();
                }

                std::string value() const
                {
                    return line_.substr(pos_eq_ + 1);
                }

                std::string const & to_string() const
                {
                    return line_;
                }

                boost::system::error_code from_string(
                    std::string const & str)
                {
                    std::string::size_type pos_eq = 
                        str.find('=');
                    if (pos_eq < str.size()) {
                        pos_eq_ = pos_eq;
                        line_ = str;
                        return framework::system::logic_error::succeed;
                    } else {
                        return framework::system::logic_error::invalid_argument;
                    }
                }

            private:
                std::string line_;
                std::string::size_type pos_eq_;
            };

            typedef std::vector<Parameter> param_map;

            typedef std::vector<Parameter>::iterator param_iterator;

            typedef std::vector<Parameter>::const_iterator param_const_iterator;

        public:
            param_iterator param_begin()
            {
                return params_.begin();
            }
 
            param_iterator param_end()
            {
                return params_.end();
            }
 
            param_const_iterator param_begin() const
            {
                return params_.begin();
            }
 
            param_const_iterator param_end() const
            {
                return params_.end();
            }
 
        private:
            bool is_valid_;
            std::string protocol_;
            std::string user_;
            std::string password_;
            std::string host_;
            std::string svc_;
            std::string path_;
            param_map params_;
            std::string anchor_;
        };

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_URL_H_
