// Sha1.cpp

#include "framework/Framework.h"
#include "framework/string/Sha1.h"
#include "framework/system/LogicError.h"
#include "framework/string/Base16.h"
using namespace framework::system::logic_error;

using namespace boost::system;

#ifndef FRAMEWORK_WITH_LIB_SSL
#  ifdef BOOST_BIG_ENDIAN
#    define WORDS_BIGENDIAN
#  endif
#  include "framework/string/detail/sha1.i"
namespace framework { namespace string { namespace detail {
    typedef framework_string_SHA1_CTX SHA1_CTX;
    void SHA1_Init(SHA1_CTX * context) { framework_string_SHA1Init(context); }
    void SHA1_Update(SHA1_CTX * context, unsigned char* data, boost::uint32_t len) 
	{ framework_string_SHA1Update(context, data, len); }
    void SHA1_Final(unsigned char digest[20], SHA1_CTX* context)
	{ framework_string_SHA1Final(digest, context); }
} } }
#elif (defined BOOST_WINDOWS_API)
#  if _MSC_VER > 1300
#    include <atlcomcli.h>
#  endif
#  include <atlcrypt.h>
namespace framework { namespace string { namespace detail {
    struct SHA1_CTX
    {
        CCryptProv ctx;
        CCryptSHA1Hash alg;
    };
    void SHA1_Init(SHA1_CTX * context)
    {
        if (FAILED(context->ctx.Initialize(PROV_RSA_FULL, NULL, NULL, 0)))
        {
            context->ctx.InitCreateKeySet(PROV_RSA_FULL, NULL, NULL, 0);
        }
        HRESULT hr = context->alg.Initialize(context->ctx);
        assert(SUCCEEDED(hr));
    }
    void SHA1_Update(SHA1_CTX * context, unsigned char* data, boost::uint32_t len)
    {
        HRESULT hr = context->alg.AddData(data, len);
        assert(SUCCEEDED(hr));
    }
    void SHA1_Final(unsigned char digest[20], SHA1_CTX* context)
    {
        DWORD len = 20;
        HRESULT hr = context->alg.GetValue(digest, &len);
        assert(SUCCEEDED(hr));
    }
} } }
#else
#  include <openssl/sha.h>
namespace framework { namespace string { namespace detail {
    using ::SHA_CTX;
    using ::SHA_Init;
    using ::SHA_Update;
    using ::SHA_Final;
#  define SHA1_CTX SHA_CTX
#  define SHA1_Init SHA_Init
#  define SHA1_Update SHA_Update
#  define SHA1_Final SHA_Final
} } }
#endif

namespace framework
{
    namespace string
    {

        namespace detail
        {
            struct Sha1Ctx
                : SHA1_CTX
            {
                boost::uint8_t digest[20];
            };
        }

        Sha1::Sha1()
        {
            ctx_ = new detail::Sha1Ctx;
            init();
        }

        Sha1::~Sha1()
        {
            if (ctx_)
                delete ctx_;
        }

        void Sha1::init()
        {
            detail::SHA1_Init(ctx_);
        }

        void Sha1::update(
            boost::uint8_t const * buf, 
            size_t len)
        {
            detail::SHA1_Update(ctx_, (unsigned char *)buf, len);
        }

        void Sha1::final()
        {
            detail::SHA1_Final(ctx_->digest, ctx_);
        }

        boost::uint8_t * Sha1::digest() const
        {
            return ctx_->digest;
        }

        std::string Sha1::to_string() const
        {
            char const * bytes = (char const *)&ctx_->digest;
            return Base16::encode(std::string(bytes, 16));
        }

        error_code Sha1::from_string(
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

        Sha1::bytes_type Sha1::to_bytes() const
        {
            bytes_type bytes;
            memcpy(bytes.elems, ctx_->digest, sizeof(ctx_->digest));
            return bytes;
        }

        void Sha1::from_bytes(
            bytes_type const & bytes)
        {
            memcpy(ctx_->digest, bytes.elems, sizeof(ctx_->digest));
        }

    } // namespace string
} // namespace framework

