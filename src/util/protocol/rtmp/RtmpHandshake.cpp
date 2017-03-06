// RtmpHandshake.cpp

#include "util/Util.h"
#include "util/protocol/rtmp/RtmpHandshake.h"

#include <framework/string/Sha256.h>
#include <framework/string/Hmac.hpp>

namespace util
{
    namespace protocol
    {

        RtmpHandshake::RtmpHandshake()
            : has_digest_(true)
            , digest_first_(true)
        {
        }

        RtmpHandshake::~RtmpHandshake()
        {
        }

        static void assgin_rand(
            boost::uint8_t & v)
        {
            v = (boost::uint8_t)rand();
        }

        static boost::uint8_t genuineFMSKey[] = {
            0x47, 0x65, 0x6e, 0x75, 0x69, 0x6e, 0x65, 0x20,
            0x41, 0x64, 0x6f, 0x62, 0x65, 0x20, 0x46, 0x6c,
            0x61, 0x73, 0x68, 0x20, 0x4d, 0x65, 0x64, 0x69,
            0x61, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72,
            0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Media Server 001
            0xf0, 0xee, 0xc2, 0x4a, 0x80, 0x68, 0xbe, 0xe8,
            0x2e, 0x00, 0xd0, 0xd1, 0x02, 0x9e, 0x7e, 0x57,
            0x6e, 0xec, 0x5d, 0x2d, 0x29, 0x80, 0x6f, 0xab,
            0x93, 0xb8, 0xe6, 0x36, 0xcf, 0xeb, 0x31, 0xae
        }; // 68

        static boost::uint8_t genuineFPKey[] = {
            0x47, 0x65, 0x6E, 0x75, 0x69, 0x6E, 0x65, 0x20,
            0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x46, 0x6C,
            0x61, 0x73, 0x68, 0x20, 0x50, 0x6C, 0x61, 0x79,
            0x65, 0x72, 0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Player 001
            0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8,
            0x2E, 0x00, 0xD0, 0xD1, 0x02, 0x9E, 0x7E, 0x57,
            0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
            0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
        }; // 62

        void RtmpHandshake::make_c01(
            StreamBuffer & buf)
        {
            boost::uint8_t * c01 = boost::asio::buffer_cast<boost::uint8_t *>(buf.prepare(1 + HANDSHAKE_SIZE));
            *c01++ = 3; // version
            std::for_each(c01, c01 + HANDSHAKE_SIZE, assgin_rand);
            if (has_digest_) {
                c01[4] = 10;
                c01[5] = 0;
                c01[6] = 12;
                c01[7] = 2;
                boost::uint32_t offset = digest_offset(c01, digest_first_);
                make_digest(c01, offset, genuineFPKey, 30, digest_);
                memcpy(c01 + offset, digest_, 32);
            } else {

            }
            buf.commit(1 + HANDSHAKE_SIZE);
        }

        void RtmpHandshake::make_c2(
            StreamBuffer & buf)
        {
            boost::uint8_t * c2 = boost::asio::buffer_cast<boost::uint8_t *>(buf.prepare(HANDSHAKE_SIZE));
            std::for_each(c2, c2 + HANDSHAKE_SIZE, assgin_rand);
            if (has_digest_) {
                make_digest2(c2, digest_, genuineFPKey, 62, c2 + HANDSHAKE_SIZE - 32);
            } else {
            }
            buf.commit(HANDSHAKE_SIZE);
        }

        void RtmpHandshake::make_s012(
            StreamBuffer & buf)
        {
            boost::uint8_t * s012 = boost::asio::buffer_cast<boost::uint8_t *>(buf.prepare(1 + HANDSHAKE_SIZE * 2));
            *s012++ = 3; // version
            std::for_each(s012, s012 + HANDSHAKE_SIZE * 2, assgin_rand);
            if (has_digest_) {
                boost::uint32_t offset = digest_offset(s012, digest_first_);
                make_digest(s012, offset, genuineFMSKey, 36, s012 + offset);
                s012 += HANDSHAKE_SIZE;
                make_digest2(s012, digest_, genuineFMSKey, 68, s012 + HANDSHAKE_SIZE - 32);
            }
            buf.commit(1 + HANDSHAKE_SIZE * 2);
        }

        bool RtmpHandshake::check_c01(
            StreamBuffer const & buf)
        {
            boost::uint8_t const * c01 = boost::asio::buffer_cast<boost::uint8_t const *>(buf.data());
            ++c01;
            if (c01[4] == 0 && c01[5] == 0 && c01[6] == 0 && c01[7] == 0) {
                has_digest_ = false;
                return true;
            }
            boost::uint32_t offset = 0;
            offset = digest_offset(c01, false);
            make_digest(c01, offset, genuineFPKey, 30, digest_);
            if (memcmp(digest_, c01 + offset, 32) == 0) {
                has_digest_ = true;
                digest_first_ = false;
                offset = dhkey_offset(c01, false);
                memcpy(dhkey_, c01 + offset, 128);
                return true;
            }
            offset = digest_offset(c01, true);
            make_digest(c01, offset, genuineFPKey, 30, digest_);
            if (memcmp(digest_, c01 + offset, 32) == 0) {
                has_digest_ = true;
                digest_first_ = true;
                offset = dhkey_offset(c01, true);
                memcpy(dhkey_, c01 + offset, 128);
                return true;
            }
            return false;
        }

        bool RtmpHandshake::check_s012(
            StreamBuffer const & buf)
        {
            boost::uint8_t const * s012 = boost::asio::buffer_cast<boost::uint8_t const *>(buf.data());
            ++s012;
            if (!has_digest_) {
                return true;
            }
            boost::uint32_t offset = 0;
            offset = digest_offset(s012, digest_first_);
            make_digest(s012, offset, genuineFMSKey, 36, digest_);
            if (memcmp(digest_, s012 + offset, 32) == 0) {
                offset = dhkey_offset(s012, false);
                memcpy(dhkey_, s012 + offset, 128);
            } else {
                return false;
            }
            return true;
        }

        bool RtmpHandshake::check_c2(
            StreamBuffer const & buf)
        {
            return false;
        }

        boost::uint32_t RtmpHandshake::digest_offset(
            boost::uint8_t const * data, 
            bool digest_first)
        {
            boost::uint32_t digest_offset = 0;
            if (digest_first) {
                digest_offset = data[8] + data[9] + data[10] + data[11];
                digest_offset %= 728;
                digest_offset += 12;
            } else {
                digest_offset = data[772] + data[773] + data[774] + data[775];
                digest_offset %= 728;
                digest_offset += 776;
            }
            return digest_offset;
        }

        boost::uint32_t RtmpHandshake::dhkey_offset(
            boost::uint8_t const * data, 
            bool digest_first)
        {
            boost::uint32_t key_offset = 0;
            if (digest_first) {
                key_offset = data[1532] + data[1533] + data[1534] + data[1535];
                key_offset %= 632;
                key_offset += 772;
            } else {
                key_offset = data[768] + data[769] + data[770] + data[771];
                key_offset %= 632;
                key_offset += 8;
            }
            return key_offset;
        }

        void RtmpHandshake::make_digest(
            boost::uint8_t const * data, 
            boost::uint32_t offset, 
            boost::uint8_t const * key, 
            boost::uint32_t key_len, 
            boost::uint8_t * out)
        {
            typedef framework::string::Hmac<framework::string::Sha256> HmacSha256;
            HmacSha256 hmac(key, key_len);
            hmac.update(data, offset);
            hmac.update(data + offset + 32, 1536 - (offset + 32));
            hmac.final();
            memcpy(out, hmac.digest(), 32);
        }

        void RtmpHandshake::make_digest2(
            boost::uint8_t const * data, 
            boost::uint8_t const * chalange_key, 
            boost::uint8_t const * key, 
            boost::uint32_t key_len, 
            boost::uint8_t * out)
        {
            typedef framework::string::Hmac<framework::string::Sha256> HmacSha256;

            HmacSha256 hmac(key, key_len);
            hmac.update(chalange_key, 32);
            hmac.final();

            HmacSha256 hmac2(hmac.digest(), 32);
            hmac2.update(data, 1536 - 32);
            hmac2.final();

            memcpy(out, hmac2.digest(), 32);
        }

    } // namespace protocol
} // namespace util
