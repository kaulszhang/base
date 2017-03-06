// Algorithm.h

#ifndef _FRAMEWORK_STRING_ALGORITHM_H_
#define _FRAMEWORK_STRING_ALGORITHM_H_

#include <boost/static_assert.hpp>

namespace framework
{
    namespace string
    {

        void trim(
            std::string & str);

        typedef std::pair<
            std::string::size_type, 
            std::string::size_type
        > string_limit;

        void trim(
            std::string const & str, 
            string_limit & limit);

        inline std::string truncate(
            std::string const & url, 
            size_t buffer_size)
        {
            if (buffer_size <= 4)
                return url.substr(0, buffer_size - 1);
            if (url.length() + 1 <= buffer_size)
                return url;
            size_t trunc_length = (buffer_size - 4) / 2;
            return url.substr(0, trunc_length) + "..." + url.substr(url.length() - trunc_length);
        }

        template<typename ByteType>
        inline void truncate_to(
            std::string const & url, 
            ByteType * buffer, 
            size_t buffer_size)
        {
            BOOST_STATIC_ASSERT(sizeof(ByteType) == 1);
            std::string url_trunc = truncate(url, buffer_size);
            strcpy((char*) buffer, url_trunc.c_str());
        }

        template<typename ByteType, size_t Size>
        inline void truncate_to(
            std::string const & url, 
            ByteType(& buffer)[Size])
        {
            BOOST_STATIC_ASSERT(sizeof(ByteType) == 1);
            truncate_to(url, buffer, Size);
        }

        bool strncasecmp(std::string const & l, std::string const & r);

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_ALGORITHM_H_
