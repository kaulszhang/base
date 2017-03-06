// Hmac.cpp

#include "framework/string/Hmac.h"
#include "framework/system/LogicError.h"
#include "framework/string/Base16.h"
using namespace framework::system::logic_error;

using namespace boost::system;

namespace framework
{
    namespace string
    {

        // HMAC (K,m) = H ((K ¨’ opad) ¡Î H ((K ¨’ ipad) ¡Î m))

        struct hash_no_init;

        namespace detail
        {
            template <
                typename Hash
            >
            struct HmacCtx
            {
                HmacCtx()
                    : hash((hash_no_init *)NULL)
                {
                    key_block.assign(0);
                }

                Hash hash;
                typename Hmac<Hash>::block_bytes_type key_block;
                typename Hmac<Hash>::bytes_type digest;
            };
        }

        template <
            typename Hash
        >
        Hmac<Hash>::Hmac()
        {
            ctx_ = new detail::HmacCtx<Hash>;
            init(NULL, 0);
        }

        template <
            typename Hash
        >
        Hmac<Hash>::Hmac(
            boost::uint8_t const * key, 
            size_t len)
        {
            ctx_ = new detail::HmacCtx<Hash>;
            init(key, len);
        }

        template <
            typename Hash
        >
        Hmac<Hash>::Hmac(
            std::vector<boost::uint8_t> const & key)
        {
            ctx_ = new detail::HmacCtx<Hash>;
            init(key.empty() ? NULL : &key.at(0), key.size());
        }

        template <
            typename Hash
        >
        Hmac<Hash>::~Hmac()
        {
            if (ctx_)
                delete ctx_;
        }

        template <
            typename Hash
        >
        void Hmac<Hash>::init(
            boost::uint8_t const * key, 
            size_t len)
        {
            ctx_->hash.init();
            if (len > block_bytes_type::static_size) {
                ctx_->digest = Hash::apply(key, len);
                memcpy(ctx_->key_block.data(), ctx_->digest.data(), ctx_->digest.size());
            } else {
                memcpy(ctx_->key_block.data(), key, len);
            }
            block_bytes_type i_key_pad;
            for (size_t i = 0; i < i_key_pad.size(); ++i) {
                i_key_pad[i] = ctx_->key_block[i] ^ 0x36;
            }
            ctx_->hash.update(i_key_pad.data(), i_key_pad.size());
        }

        template <
            typename Hash
        >
        void Hmac<Hash>::update(
            boost::uint8_t const * buf, 
            size_t len)
        {
            ctx_->hash.update((unsigned char *)buf, len);
        }

        template <
            typename Hash
        >
        void Hmac<Hash>::final()
        {
            ctx_->hash.final();
            ctx_->digest = ctx_->hash.to_bytes();
            ctx_->hash.init();
            block_bytes_type o_key_pad;
            for (size_t i = 0; i < o_key_pad.size(); ++i) {
                o_key_pad[i] = ctx_->key_block[i] ^ 0x5c;
            }
            ctx_->hash.update(o_key_pad.data(), o_key_pad.size());
            ctx_->hash.update(ctx_->digest.data(), ctx_->digest.size());
            ctx_->hash.final();
            ctx_->digest = ctx_->hash.to_bytes();
        }

        template <
            typename Hash
        >
        boost::uint8_t * Hmac<Hash>::digest() const
        {
            return ctx_->digest.data();
        }

        template <
            typename Hash
        >
        typename Hash::bytes_type Hmac<Hash>::apply(
            boost::uint8_t const * key, 
            size_t key_len, 
            boost::uint8_t const * buf, 
            size_t len)
        {
            Hmac<Hash> hmac(key, key_len);
            hmac.update(buf, len);
            hmac.final();
            return hmac.to_bytes();
        }

        template <
            typename Hash
        >
        std::string Hmac<Hash>::to_string() const
        {
            char const * bytes = (char const *)&ctx_->digest->data();
            return Base16::encode(std::string(bytes, bytes_type::static_size));
        }

        template <
            typename Hash
        >
        error_code Hmac<Hash>::from_string(
            std::string const & str)
        {
            std::string hash = Base16::decode(str);
            if (hash.size() == sizeof(bytes_type::static_size)) {
                memcpy(&ctx_->digest->data(), hash.c_str(), bytes_type::static_size);
                return succeed;
            } else {
                return invalid_argument;
            }
        }

        template <
            typename Hash
        >
        typename Hash::bytes_type Hmac<Hash>::to_bytes() const
        {
            return ctx_->digest;
        }

        template <
            typename Hash
        >
        void Hmac<Hash>::from_bytes(
            bytes_type const & bytes)
        {
           ctx_->digest = bytes;
        }

    } // namespace string
} // namespace framework

