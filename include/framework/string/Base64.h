// Base64.h

#ifndef _FRAMEWORK_STRING_BASE64_H_
#define _FRAMEWORK_STRING_BASE64_H_

namespace framework
{
    namespace string
    {

        class Base64
        {
        public:
            static size_t encode(
                char const * str_src, 
                size_t size_src, 
                char * str_dst, 
                size_t size_dst);

            static size_t decode(
                char const * str_src, 
                size_t size_src, 
                char * str_dst, 
                size_t size_dst);

            static std::string encode(
                char const * str, 
                size_t size);

            static std::string decode(
                char const * str, 
                size_t size);

            static std::string encode(
                std::string const & str)
            {
                return encode(str.c_str(), str.size());
            }

            static std::string decode(
                std::string const & str)
            {
                return decode(str.c_str(), str.size());
            }

            static std::string encode(
                boost::uint8_t const * data, 
                size_t size)
            {
                return encode((char const *)data, size);
            }

            static std::string decode(
                char const * str);

        };

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_BASE64_H_
