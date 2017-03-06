// Url.cpp

#include "framework/Framework.h"
#include "framework/string/Url.h"
#include "framework/string/StringToken.h"
#include "framework/string/Slice.h"
#include "framework/string/Join.h"
using namespace framework::system::logic_error;

#include <boost/regex.hpp>
using namespace boost::system;

#include <iterator>
#include <memory>

namespace framework
{
    namespace string
    {

        Url::Url(
            std::string const & url)
        {
            from_string(url);
        }

        std::string Url::to_string() const
        {
            if (!is_valid()) {
                return "invalid url";
            }
            std::string url = protocol_ + "://";
            if (!user_.empty()) {
                url += user_;
                if (!password_.empty()) {
                    url += ":";
                    url += password_;
                }
                url += "@";
            }
            url += host_;
            if (!svc_.empty()) {
                url += ":";
                url += svc_;
            }
            url += path_;
            if (!params_.empty()) {
                url += "?";
                url += join(params_.begin(), params_.end(), "&");
            }
            if (!anchor_.empty()) {
                url += "#";
                url += anchor_;
            }
            return url;
        }

        struct not_graph
        {
            bool operator()(
                char const & c)
            {
                return !isgraph((unsigned char)c);
            }
        };

        error_code Url::from_string(
            std::string const & url)
        {
#define PROTOCOL    "[a-zA-Z][a-zA-Z0-9]+"
#define USER        "[^:/@]+"
#define PASS        USER
#define HOST        "[-\\w]+(?:\\.[-\\w]+)*"
#define HOSTV6      "\\[.*\\]+"
#define PORT        "\\d+"
#define PATH        "/[^\\?#\\*\\s]*"
#define PARAM       "[-a-zA-Z0-9+&@/%=~_|!:,.;\\(\\)]*"
#define AUCHOR      PARAM

            std::string expr = "^(?:(" PROTOCOL "):/{1,2}(?:(?:(" USER ")(?::(" PASS "))?@)?(" HOST ")(?::(" PORT "))?)?)(" PATH ")(?:\\?(" PARAM "))?(?:#(" AUCHOR "))?$";

            if (url.find("://[") != std::string::npos)
            {
                //IPV6
                expr = "^(?:(" PROTOCOL "):/{1,2}(?:(?:(" USER ")(?::(" PASS "))?@)?(" HOSTV6 ")(?::(" PORT "))?)?)(" PATH ")(?:\\?(" PARAM "))?(?:#(" AUCHOR "))?$";
            }


            std::string const * p_url(&url);
            if (std::find_if(url.begin(), url.end(), not_graph()) != url.end()) {
                std::string * p = new std::string(url);
                p_url = p;
                std::replace_if(p->begin(), p->end(), not_graph(), '.');
            }

            error_code ec = succeed;
            try {
                boost::regex re(expr);
                boost::cmatch what;
                if (boost::regex_match(p_url->c_str(), what, re)) {
                    is_valid_ = true;
                    if (what[1].matched)
                        protocol_ = what[1].str();
                    if (what[2].matched)
                        user_ = what[2].str();
                    if (what[3].matched)
                        password_ = what[3].str();
                    if (what[4].matched)
                        host_ = what[4].str();
                    if (what[5].matched)
                        svc_ = what[5].str();
                    if (what[6].matched)
                        path_ = (p_url == &url) ? what[6].str() : 
                            url.substr(what[6].first - p_url->c_str(), what[6].second - what[6].first);
                    if (what[7].matched) {
                        slice<param_map::value_type>(what[7].str(), params_, "&");
                    }
                    if (what[8].matched) {
                        anchor_ = what[8].str();
                    }
                } else {
                    is_valid_ = false;
                    ec = invalid_argument;
                }
            } catch(...) {
                is_valid_ = false;
                ec = unknown_error;
            }
            if (p_url != &url)
                delete p_url;
            return ec;
        }

        std::string Url::path_all() const
        {
            std::string path = path_;
            if (!params_.empty()) {
                path += "?";
                path += join(params_.begin(), params_.end(), "&");
            }
            if (!anchor_.empty()) {
                path += "#";
                path += anchor_;
            }
            return path;
        }

        std::string Url::param(
            std::string const & key) const
        {
            param_map::const_iterator i = 
                std::find(params_.begin(), params_.end(), key);
            if (i == params_.end())
                return "";
            else
                return i->value();
        }

        std::string Url::param_or(
            std::string const & key, 
            std::string const & def) const
        {
            param_map::const_iterator i = 
                std::find(params_.begin(), params_.end(), key);
            if (i == params_.end())
                return def;
            else
                return i->value();
        }

        void Url::param(
            std::string const & key, 
            std::string const & value)
        {
            param_map::iterator i = 
                std::find(params_.begin(), params_.end(), key);
            if (i == params_.end()) {
                if (!value.empty()) {
                    params_.push_back(Parameter(key, value));
                }
            } else {
                if (value.empty()) {
                    params_.erase(i);
                } else {
                    *i = value;
                }
            }
        }

        void Url::encode()
        {
            path_ = encode(path_, "/.");
            param_map::iterator i = params_.begin();
            for (; i != params_.end(); ++i) {
                std::string v = encode(i->value(), "/.@");
                const_cast<Parameter &>(*i) = v;
            }
        }

        void Url::decode()
        {
            path_ = decode(path_);
            param_map::iterator i = params_.begin();
            for (; i != params_.end(); ++i) {
                std::string v = decode(i->value());
                const_cast<Parameter &>(*i) = v;
            }
        }

        std::string Url::encode(
            std::string const & str, 
            char const * ignore)
        {
            char const * hex_chr = "0123456789ABCDEF";
            std::string result = "";
            for (unsigned int i = 0; i < str.size(); ++i) {
                char c = str[i];
                if (('0' <= c && c <= '9')
                    || ('a' <= c && c <= 'z') 
                    || ('A' <= c && c <= 'Z') 
                    || strchr(ignore, c)) {
                        result += c;
                } else {
                    int j = (short int)c;
                    if (j < 0) {
                        j += 256;
                    }
                    result += '%';
                    result += hex_chr[(j >> 4) & 0xf];
                    result += hex_chr[j & 0xf];
                }
            }
            return result;
        } 

        std::string Url::decode(
            std::string const & str)
        {
            if (str.size() < 3)
                return str;
            std::string result = "";
            size_t i = 0;
            for (; i < str.size() - 2; i++) {
                char c = str[i];
                if (c == '%') {
                    char const h = str[++i];
                    char const l = str[++i];
                    if (h >= '0' && h <= '9') {
                        c = h - '0';
                    } else if (h >= 'A' && h <= 'F') {
                        c = h - ('A' - 10);
                    } else if (h >= 'a' && h <= 'f') {
                        c = h - ('a' - 10);
                    } else {
                        result.append(1, '%');
                        result.append(1, h);
                        continue;
                    }
                    c = c << 4;
                    if (l >= '0' && l <= '9') {
                        c |= l - '0';
                    } else if (l >= 'A' && l <= 'F') {
                        c |= l - ('A' - 10);
                    } else if (l >= 'a' && l <= 'f') {
                        c |= l - ('a' - 10);
                    } else {
                        result.append(1, '%');
                        result.append(1, h);
                        result.append(1, l);
                        continue;
                    }
                }
                result.append(1, c);
            }
            result.append(str, i, str.size() - i);
            return result;
        }

    } // namespace string
} // namespace framework
