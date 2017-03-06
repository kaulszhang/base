// Sha256.h

#ifndef _FRAMEWORK_STRING_HMAC_H_
#define _FRAMEWORK_STRING_HMAC_H_

#include <boost/array.hpp>

namespace framework
{
    namespace string
    {

        namespace detail
        {
            template <
                typename Hash
            >
            struct HmacCtx;
        }

        template <
            typename Hash
        >
        class Hmac
        {
        public:
            typedef boost::array<boost::uint8_t, Hash::block_size> block_bytes_type;

            typedef typename Hash::bytes_type bytes_type;

        public:
            Hmac();

            Hmac(
                boost::uint8_t const * key, 
                size_t len);

            Hmac(
                std::vector<boost::uint8_t> const & key);

            ~Hmac();

        public:
            void init(
                boost::uint8_t const * key, 
                size_t len);

            void update(
                boost::uint8_t const * buf, 
                size_t len);

            void final();

            boost::uint8_t * digest() const;

        public:
            static bytes_type apply(
                boost::uint8_t const * key, 
                size_t key_len, 
                boost::uint8_t const * buf, 
                size_t len);

        public:
            bytes_type to_bytes() const;

            void from_bytes(
                bytes_type const & str);

        public:
            std::string to_string() const;

            boost::system::error_code from_string(
                std::string const & str);

        private:
            detail::HmacCtx<Hash> * ctx_;
        };

        template <
            typename Hash
        >
        inline typename Hmac<Hash>::bytes_type hmac(
            boost::uint8_t const * key, 
            size_t key_len, 
            boost::uint8_t const * buf, 
            size_t len)
        {
            return Hmac<Hash>::apply(key, key_len, buf, len);
        }

        template <
            typename Hash
        >
        inline typename Hmac<Hash>::bytes_type hmac(
            std::string const & key, 
            std::string const & data)
        {
            return Hmac<Hash>::apply(
                (boost::uint8_t const *)key.c_str(), key.size(), 
                (boost::uint8_t const *)data.c_str(), data.size());
        }

        template <
            typename Hash
        >
        inline typename Hmac<Hash>::bytes_type hmac(
            boost::uint8_t const * buf, 
            size_t len)
        {
            return Hmac<Hash>::apply(NULL, 0, buf, len);
        }

        template <
            typename Hash
        >
        inline typename Hmac<Hash>::bytes_type hmac(
            std::string const & data)
        {
            return Hmac<Hash>::apply(
                NULL, 0, 
                (boost::uint8_t const *)data.c_str(), data.size());
        }

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_SHA256_H_
