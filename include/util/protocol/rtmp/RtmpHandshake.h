// RtmpHandshake.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_HANDSHAKE_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_HANDSHAKE_H_

#include <util/buffers/StreamBuffer.h>

namespace util
{
    namespace protocol
    {

        class RtmpHandshake
        {
        public:
            static boost::uint32_t const RANDOM_SIZE = 1528;
            static boost::uint32_t const HANDSHAKE_SIZE = 8 + RANDOM_SIZE;

            typedef util::buffers::StreamBuffer<boost::uint8_t> StreamBuffer;

        public:
            RtmpHandshake();

            ~RtmpHandshake();

        public:
            void make_c01(
                StreamBuffer & buf);
            
            void make_c2(
                StreamBuffer & out);

            void make_s012(
                StreamBuffer & out);

        public:
            bool check_c01(
                StreamBuffer const & buf);

            bool check_s012(
                StreamBuffer const & buf);

            bool check_c2(
                StreamBuffer const & buf);

        private:
            static boost::uint32_t digest_offset(
                boost::uint8_t const * data, 
                bool digest_first);

            static boost::uint32_t dhkey_offset(
                boost::uint8_t const * data, 
                bool digest_first);

            static void make_digest(
                boost::uint8_t const * data, 
                boost::uint32_t offset, 
                boost::uint8_t const * key, 
                boost::uint32_t key_len, 
                boost::uint8_t * out);

            static void make_digest2(
                boost::uint8_t const * data, 
                boost::uint8_t const * chalange_key, 
                boost::uint8_t const * key, 
                boost::uint32_t key_len, 
                boost::uint8_t * out);

        private:
            bool has_digest_;
            bool digest_first_;
            boost::uint8_t dhkey_[128]; // client dh key
            boost::uint8_t digest_[32]; // client digest
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_HANDSHAKE_H_
