// Uuid.cpp

#include "framework/Framework.h"
#include "framework/string/Uuid.h"
#include "framework/string/Base16.h"
#include "framework/system/LogicError.h"
#include "framework/system/BytesOrder.h"
using namespace framework::system::logic_error;
using namespace framework::system;

using namespace boost::system;

#ifndef FRAMEWORK_WITH_LIB_UUID
#include "framework/string/detail/uuid.c"
namespace framework { namespace string { namespace detail {
    using ::uuid_t;
    using ::uuid_generate;
} } }
#elif (defined BOOST_WINDOWS_API)
#include <Objbase.h>
#pragma comment(lib, "Ole32")
namespace framework { namespace string { namespace detail {
    using ::uuid_t;
    void uuid_generate(uuid_t & out) {
        HRESULT hr = ::CoCreateGuid(&out);
        (void)hr;
        assert(S_OK == hr);
    }
} } }
#else
#  include <uuid/uuid.h>
namespace framework { namespace string { namespace detail {
    using ::uuid_t;
    using ::uuid_generate;
} } }
#endif

namespace framework
{
    namespace string
    {
        Uuid const Uuid::Null;

        std::string Uuid::to_string() const
        {
            bytes_type bytes = to_big_endian_bytes();
            return Base16::encode(std::string((char const *)bytes.elems, sizeof(UUID)));
        }

        error_code Uuid::from_string(
            std::string const & str)
        {
            std::string uuid = Base16::decode(str);
            if (str.length() != 32) {
                return invalid_argument;
            }
            bytes_type bytes;
            memcpy(bytes.elems, &uuid[0], sizeof(UUID));
            from_big_endian_bytes(bytes);
            return succeed;
        }

        Uuid::bytes_type Uuid::to_bytes() const
        {
            bytes_type bytes;
            memcpy(bytes.elems, &uuid_, sizeof(uuid_));
            return bytes;
        }

        void Uuid::from_bytes(
            bytes_type const & bytes)
        {
            memcpy(&uuid_, bytes.elems, sizeof(uuid_));
        }

        Uuid::bytes_type Uuid::to_little_endian_bytes() const
        {
            bytes_type bytes;
            UUID uuid = uuid_;
            uuid.Data1 = BytesOrder::host_to_little_endian_long(uuid.Data1);
            uuid.Data2 = BytesOrder::host_to_little_endian_short(uuid.Data2);
            uuid.Data3 = BytesOrder::host_to_little_endian_short(uuid.Data3);
            memcpy(bytes.elems, &uuid, sizeof(uuid));
            return bytes;
        }

        void Uuid::from_little_endian_bytes(
            bytes_type const & bytes)
        {
            UUID uuid;
            memcpy(&uuid, bytes.elems, sizeof(uuid));
            uuid.Data1 = BytesOrder::little_endian_to_host_long(uuid.Data1);
            uuid.Data2 = BytesOrder::little_endian_to_host_short(uuid.Data2);
            uuid.Data3 = BytesOrder::little_endian_to_host_short(uuid.Data3);
            uuid_ = uuid;
        }

        Uuid::bytes_type Uuid::to_big_endian_bytes() const
        {
            bytes_type bytes;
            UUID uuid = uuid_;
            uuid.Data1 = BytesOrder::host_to_big_endian_long(uuid.Data1);
            uuid.Data2 = BytesOrder::host_to_big_endian_short(uuid.Data2);
            uuid.Data3 = BytesOrder::host_to_big_endian_short(uuid.Data3);
            memcpy(bytes.elems, &uuid, sizeof(uuid));
            return bytes;
        }

        void Uuid::from_big_endian_bytes(
            bytes_type const & bytes)
        {
            UUID uuid;
            memcpy(&uuid, bytes.elems, sizeof(uuid));
            uuid.Data1 = BytesOrder::big_endian_to_host_long(uuid.Data1);
            uuid.Data2 = BytesOrder::big_endian_to_host_short(uuid.Data2);
            uuid.Data3 = BytesOrder::big_endian_to_host_short(uuid.Data3);
            uuid_ = uuid;
        }

        void Uuid::generate()
        {
            detail::uuid_t uuid;
            detail::uuid_generate(uuid);
            memcpy(&uuid_, &uuid, sizeof(uuid_));
        }

    } // namespace string
} // namespace framework
