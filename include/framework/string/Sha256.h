// Sha256.h

#ifndef _FRAMEWORK_STRING_SHA256_H_
#define _FRAMEWORK_STRING_SHA256_H_

#include <boost/array.hpp>

namespace framework
{
    namespace string
    {

        struct hash_no_init;

        namespace detail
        {
            struct Sha256Ctx;
        }

        class Sha256
        {
        public:
            static size_t const block_size = 64; // in bytes

            static size_t const output_size = 32; // in bytes

            typedef boost::array<boost::uint8_t, output_size> bytes_type;

        public:
            Sha256();

            Sha256(
                hash_no_init *);

            ~Sha256();

        public:
            void init();

            void update(
                boost::uint8_t const * buf, 
                size_t len);

            void final();

            boost::uint8_t * digest() const;

        public:
            static bytes_type apply(
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
            detail::Sha256Ctx * ctx_;
        };

        inline Sha256::bytes_type sha256(
            boost::uint8_t const * buf, 
            size_t len)
        {
            return Sha256::apply(buf, len);
        }

        inline Sha256::bytes_type sha256(
            std::string const & data)
        {
            return Sha256::apply((boost::uint8_t const *)data.c_str(), data.size());
        }

        inline Sha256::bytes_type sha256(
            std::vector<boost::uint8_t> const & data)
        {
            return Sha256::apply(data.empty() ? NULL : &data.at(0), data.size());
        }

        template <
            size_t N
        >
        inline Sha256::bytes_type sha256(
            boost::uint8_t const (& data)[N])
        {
            return Sha256::apply(data, N);
        }

        template <
            size_t N
        >
        inline Sha256::bytes_type sha256(
            boost::array<boost::uint8_t, N> const & data)
        {
            return Sha256::apply(data.data(), data.size());
        }

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_SHA256_H_
