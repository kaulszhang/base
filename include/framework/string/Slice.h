// Slice.h

#ifndef _FRAMEWORK_STRING_SLICE_H_
#define _FRAMEWORK_STRING_SLICE_H_

#include <framework/system/LogicError.h>
#include <framework/string/StringToken.h>
#include <framework/string/Parse.h>
#include <algorithm> 

namespace framework
{
    namespace string
    {

        template<typename _Ty>
        inline boost::system::error_code slice(
            std::string const & str, 
            std::vector<_Ty>& out, 
            std::string const & delim = ",", 
            std::string const & prefix = "", 
            std::string const & suffix = "")
        {
            using framework::system::logic_error::succeed;
            using framework::system::logic_error::invalid_argument;
            using framework::system::logic_error::no_more_item;
            
            out.clear();

            if (str.size() < prefix.size() + suffix.size()
                || str.compare(0, prefix.size(), prefix) 
                || str.compare(str.size() - suffix.size(), suffix.size(), suffix))
                return invalid_argument;
            
            StringToken st(str, delim, false, prefix.size(), str.size() - suffix.size());
            std::string item;
            boost::system::error_code ec(succeed);

            while (!st.next_token(item, ec)) {
                _Ty v;
                if (!(ec = parse2(item, v))) {
                    typename std::vector<_Ty>::iterator i; 
                     i =  std::find(out.begin(), out.end(), (std::string)v);
                    if (i != out.end()) {
                        out.erase(i);
                    }
                    out.push_back(v);
                } else {
                    break;
                }
            }
            if (ec == no_more_item)
                ec = succeed;
            return ec;
        }

        template<typename _Ty, typename _It>
        inline boost::system::error_code slice(
            std::string const & str, 
            _It out, 
            std::string const & delim = ",", 
            std::string const & prefix = "", 
            std::string const & suffix = "")
        {
            using framework::system::logic_error::succeed;
            using framework::system::logic_error::invalid_argument;
            using framework::system::logic_error::no_more_item;

            if (str.size() < prefix.size() + suffix.size()
                || str.compare(0, prefix.size(), prefix) 
                || str.compare(str.size() - suffix.size(), suffix.size(), suffix))
                return invalid_argument;
            StringToken st(str, delim, false, prefix.size(), str.size() - suffix.size());
            std::string item;
            boost::system::error_code ec(succeed);
            while (!st.next_token(item, ec)) {
                _Ty v;
                if (!(ec = parse2(item, v))) {
                    *out = v;
                    ++out;
                } else {
                    break;
                }
            }
            if (ec == no_more_item)
                ec = succeed;
            return ec;
        }

        template <
            typename _Ty
        >
        inline boost::system::error_code map_find(
            std::string const & str, 
            std::string const & key, 
            _Ty & t, 
            std::string const & item_delim = ",", 
            std::string const & pair_delim = "=")
        {
            using framework::system::logic_error::item_not_exist;
            std::string::size_type key_pos = 
                str.find(key + pair_delim);
            if (key_pos == std::string::npos) {
                return item_not_exist;
            }
            key_pos += key.size() + pair_delim.size();
            std::string::size_type item_end = 
                str.find(item_delim, key_pos);
            if (item_end == std::string::npos) {
                item_end = str.size();
            }
            return parse2(str.substr(key_pos, item_end - key_pos), t);
        }

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_SLICE_H_
