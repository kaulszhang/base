// Sha1.h

#ifndef _FRAMEWORK_STRING_SHA1_H_
#define _FRAMEWORK_STRING_SHA1_H_

#include <boost/array.hpp>

namespace framework
{
    namespace string
    {

        namespace detail
        {
            struct Sha1Ctx;
        }

        class Sha1
        {
        public:
            typedef boost::array<boost::uint8_t, 16> bytes_type;

        public:
            Sha1();

            ~Sha1();

        public:
            void init();

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

        private:
            detail::Sha1Ctx * ctx_;
        };

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_SHA1_H_
