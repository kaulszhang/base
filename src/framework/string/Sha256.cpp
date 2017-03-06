// Sha256.cpp

#include "framework/Framework.h"
#include "framework/string/Sha256.h"
#include "framework/system/LogicError.h"
#include "framework/string/Base16.h"
using namespace framework::system::logic_error;

using namespace boost::system;

#ifndef FRAMEWORK_WITH_LIB_SSL
#  ifdef BOOST_BIG_ENDIAN
#    define WORDS_BIGENDIAN
#  endif
#  include "framework/string/detail/sha256.i"
namespace framework { namespace string { namespace detail {
    typedef framework_string_SHA256Context SHA256_CTX;
    void SHA256_Init(SHA256_CTX * context) { framework_string_SHA256Init(context); }
    void SHA256_Update(SHA256_CTX * context, unsigned char* data, boost::uint32_t len) 
    { framework_string_SHA256Update(context, data, len); }
    void SHA256_Final(unsigned char digest[32], SHA256_CTX* context)
    { framework_string_SHA256Final(context, digest); }
} } }
#elif (defined BOOST_WINDOWS_API)
#  if _MSC_VER > 1300
#    include <atlcomcli.h>
#  endif
#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WS03
#  include <atlcrypt.h>

class CCryptSHA256Hash : public CCryptHash
{
public:

    HRESULT Initialize(CCryptProv &Prov, LPCTSTR szText = NULL) throw()
    {
        ATLASSUME(m_hHash == NULL);

        if (!CryptCreateHash(Prov.GetHandle(), CALG_SHA_256, 0, 0, &m_hHash))
        {
            return AtlHresultFromLastError();
        }
        if (szText!=NULL)
            return AddString(szText);
        else return S_OK;
    }
}; // class CCryptSHA256Hash

namespace framework { namespace string { namespace detail {

    struct SHA256_CTX
    {
        CCryptProv ctx;
        CCryptSHA256Hash alg;
    };
    void SHA256_Init(SHA256_CTX * context)
    {
        if (FAILED(context->ctx.Initialize(PROV_RSA_AES, NULL, NULL, 0)))
        {
            context->ctx.InitCreateKeySet(PROV_RSA_AES, NULL, NULL, 0);
        }
        HRESULT hr = context->alg.Initialize(context->ctx);
        assert(SUCCEEDED(hr));
    }
    void SHA256_Update(SHA256_CTX * context, unsigned char* data, boost::uint32_t len)
    {
        HRESULT hr = context->alg.AddData(data, len);
        assert(SUCCEEDED(hr));
    }
    void SHA256_Final(unsigned char digest[32], SHA256_CTX* context)
    {
        DWORD len = 32;
        HRESULT hr = context->alg.GetValue(digest, &len);
        assert(SUCCEEDED(hr));
        context->alg.Uninitialize();
        context->ctx.Uninitialize();
    }
} } }
#else
#  include <openssl/sha.h>
namespace framework { namespace string { namespace detail {
    using ::SHA256_CTX;
    using ::SHA256_Init;
    using ::SHA256_Update;
    using ::SHA256_Final;
} } }
#endif

namespace framework
{
    namespace string
    {

        namespace detail
        {
            struct Sha256Ctx
                : SHA256_CTX
            {
                boost::uint8_t digest[32];
            };
        }

        Sha256::Sha256()
        {
            ctx_ = new detail::Sha256Ctx;
            init();
        }

        Sha256::Sha256(
            hash_no_init *)
        {
            ctx_ = new detail::Sha256Ctx;
        }

        Sha256::~Sha256()
        {
            if (ctx_)
                delete ctx_;
        }

        void Sha256::init()
        {
            detail::SHA256_Init(ctx_);
        }

        void Sha256::update(
            boost::uint8_t const * buf, 
            size_t len)
        {
            detail::SHA256_Update(ctx_, (unsigned char *)buf, len);
        }

        void Sha256::final()
        {
            detail::SHA256_Final(ctx_->digest, ctx_);
        }

        boost::uint8_t * Sha256::digest() const
        {
            return ctx_->digest;
        }

        Sha256::bytes_type Sha256::apply(
            boost::uint8_t const * buf, 
            size_t len)
        {
            Sha256 sha;
            sha.update(buf, len);
            sha.final();
            return sha.to_bytes();
        }

        std::string Sha256::to_string() const
        {
            char const * bytes = (char const *)&ctx_->digest;
            return Base16::encode(std::string(bytes, 32));
        }

        error_code Sha256::from_string(
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

        Sha256::bytes_type Sha256::to_bytes() const
        {
            bytes_type bytes;
            memcpy(bytes.elems, ctx_->digest, sizeof(ctx_->digest));
            return bytes;
        }

        void Sha256::from_bytes(
            bytes_type const & bytes)
        {
            memcpy(ctx_->digest, bytes.elems, sizeof(ctx_->digest));
        }

    } // namespace string
} // namespace framework

