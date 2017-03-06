// Join.h

#ifndef _FRAMEWORK_STRING_JOIN_H_
#define _FRAMEWORK_STRING_JOIN_H_

#include <framework/string/Format.h>

namespace framework
{
    namespace string
    {

        template<typename _It>
        inline void join(
            std::string & str, 
            _It first, 
            _It last, 
            std::string const & delim, 
            std::string const & prefix, 
            std::string const & suffix)
        {
            str = prefix;
            if (first != last) {
                str += format(*first++);
                for (; first != last; ++first) {
                    str += delim;
                    str += format(*first);
                }
            }
            str += suffix;
        }

        template<typename _It>
        inline std::string join(
            _It first, 
            _It last, 
            std::string const & delim = ",", 
            std::string const & prefix = "", 
            std::string const & suffix = "")
        {
            std::string str;
            join(str, first, last, delim, prefix, suffix);
            return str;
        }

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_JOIN_H_
