// StringToken.cpp

#include "framework/Framework.h"
#include "framework/string/StringToken.h"

#include <boost/system/system_error.hpp>

namespace framework
{
    namespace string
    {

        using boost::system::error_code;
        using framework::system::logic_error::succeed;
        using framework::system::logic_error::no_more_item;

        StringToken::StringToken(
            std::string const & str, 
            std::string const & delim, 
            bool skip_empty, 
            std::string::size_type beg, 
            std::string::size_type end)
            : str_(str)
            , delim_(delim)
            , skip_empty_(skip_empty)
            , pos_(beg)
            , end_(end == std::string::npos ? str.size() : end)
        {
        }

        std::string StringToken::next_token()
        {
            std::string v;
            next_token(v);
            return v;
        }

        void StringToken::next_token(
            std::string & token)
        {
            boost::system::error_code ec;
            next_token(token, ec);
            if (ec)
                throw boost::system::system_error(ec);
        }

        error_code StringToken::next_token(
            std::string & token, 
            boost::system::error_code & ec)
        {
            while(pos_ < end_) {
                std::string::size_type s = pos_;
                std::string::size_type p = str_.find(delim_, pos_);
                if (p == std::string::npos || p > end_)
                    p = end_;
                pos_ = p + delim_.size();
                if (p > s || !skip_empty_) {
                    token = str_.substr(s, p - s);
                    return ec = succeed;
                }
            };
            return ec = (no_more_item);
        }

        error_code StringToken::next_token(
            error_code & ec)
        {
            while(pos_ < end_) {
                std::string::size_type s = pos_;
                std::string::size_type p = str_.find(delim_, pos_);
                if (p == std::string::npos || p > end_)
                    p = end_;
                pos_ = p + delim_.size();
                if (p > s || !skip_empty_) {
                    return ec = succeed;
                }
            };
            return ec = no_more_item;
        }

        std::string StringToken::remain() const
        {
            if (pos_ >= end_)
                return std::string();
            return str_.substr(pos_, end_ - pos_);
        }

    } // namespace string
} // namespace framework
