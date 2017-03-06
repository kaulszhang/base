// Md5.cpp

#include "framework/Framework.h"
#include "framework/string/Md5.h"
#include "framework/string/Base16.h"
#include "framework/system/LogicError.h"
using namespace framework::system::logic_error;

using namespace boost::system;

namespace framework
{
    namespace string
    {

        // include to namespace framework::string::detail
        namespace detail
        {
#include "framework/string/detail/md5.h"
            struct Md5Ctx
                : MD5_CTX
            {
            };
        }

        Md5::Md5(
            boost::uint32_t rand)
        {
            ctx_ = new detail::Md5Ctx;
            init(rand);
        }

        Md5::~Md5()
        {
            if (ctx_)
                delete ctx_;
        }

        void Md5::init(
            boost::uint32_t rand)
        {
            detail::MD5Init(ctx_, rand);
        }

        void Md5::update(
            boost::uint8_t const * buf, 
            size_t len)
        {
            detail::MD5Update(ctx_, (unsigned char *)buf, len);
        }

        void Md5::final()
        {
            detail::MD5Final(ctx_);
        }

        boost::uint8_t * Md5::digest() const
        {
            return ctx_->digest;
        }

        std::string Md5::to_string() const
        {
            char const * bytes = (char const *)&ctx_->digest;
            return Base16::encode(std::string(bytes, 16));
        }

        error_code Md5::from_string(
            std::string const & str)
        {
            std::string md5 = Base16::decode(str);
            if (md5.size() == sizeof(ctx_->digest)) {
                memcpy((char *)&ctx_->digest, md5.c_str(), sizeof(ctx_->digest));
                return succeed;
            } else {
                return invalid_argument;
            }
        }

        Md5::bytes_type Md5::to_bytes() const
        {
            bytes_type bytes;
            memcpy(bytes.elems, ctx_->digest, sizeof(ctx_->digest));
            return bytes;
        }

        void Md5::from_bytes(
            bytes_type const & bytes)
        {
            memcpy(ctx_->digest, bytes.elems, sizeof(ctx_->digest));
        }

    } // namespace string
} // namespace framework

