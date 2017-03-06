// StringToken.cpp

#include "framework/Framework.h"
#include "framework/string/Algorithm.h"
#include <algorithm>

namespace framework
{
    namespace string
    {

        static int white_char[] = {
            0, 0, 0, 0,   0, 0, 0, 0,   1, 0, 1, 0,   0, 1, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            1, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
            0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
        };

        void trim(
            std::string & str)
        {
			std::string::size_type l = str.size();
			if (l == 0) return;
			std::string::size_type s = 0;
			while (s < l && white_char[(size_t)(unsigned char)str[s]]) s++;
			std::string::size_type e = l - 1;
			while (e >= s && white_char[(size_t)(unsigned char)str[e]]) e--;
			str = str.substr(s, e - s + 1);
        }

        void trim(
            std::string const & str, 
            string_limit & limit)
        {
            while (limit.first < limit.second && white_char[(int)str[limit.first]])
                limit.first++;
            while (limit.second > limit.first && white_char[(int)str[limit.second - 1]])
                limit.second--;
        }

        bool strncasecmp(std::string const & l, std::string const & r)        {            if ( l.size() != r.size() ) return false;            std::string left(l);            std::string right(r);            transform(left.begin(), left.end(), left.begin(), tolower);            transform(right.begin(), right.end(), right.begin(), tolower);            return (left == right);        }

    } // namespace string
} // namespace framework

