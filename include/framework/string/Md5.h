// Md5.h

#ifndef _FRAMEWORK_STRING_MD5_H_
#define _FRAMEWORK_STRING_MD5_H_

#include <boost/array.hpp>

namespace framework
{
    namespace string
    {

        namespace detail
        {
            struct Md5Ctx;
        }

        class Md5
        {
        public:
            typedef boost::array<boost::uint8_t, 16> bytes_type;

        public:
            Md5(
                boost::uint32_t rand = 0);

            ~Md5();

        public:
            void init(
                boost::uint32_t rand = 0);

            void update(
                boost::uint8_t const * buf, 
                size_t len);

            void final();

            boost::uint8_t * digest() const;

        public:
            bytes_type to_bytes() const;

            void from_bytes(
                bytes_type const & str);

        public:
            std::string to_string() const;

            boost::system::error_code from_string(
                std::string const & str);

        protected:
            detail::Md5Ctx * ctx_;
        };

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_MD5_H_
