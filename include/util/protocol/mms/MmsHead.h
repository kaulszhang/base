// MmsHead.h

#ifndef _UTIL_PROTOCOL_MMS_HEAD_H_
#define _UTIL_PROTOCOL_MMS_HEAD_H_

#include <istream>
#include <ostream>

namespace util
{
    namespace protocol
    {

        struct MmsHead
        {
        public:
            boost::uint8_t rep;                     // 必须为0x01
            boost::uint8_t ver;                     // 必须为0x00
            boost::uint8_t min_ver;                 // 必须为0x00
            boost::uint8_t pad;                     // 应该为0x00，接收端忽略
            boost::uint32_t session_id;             // 命令签名，必须为0xb00bface
            boost::uint32_t msg_len;                // 命令长度：bytes
            boost::uint32_t seal;                   // 协议类型，必须为0x20534d4d
            boost::uint32_t chunk_count;            // 当前位置到结束的块的个数 X 8 byte
            boost::uint16_t seq;                    // 序列号，每发送一次加一
            boost::uint16_t mbz;                    // 必须为0x0000
            boost::uint64_t timestamp;              // 时间戳，接收端忽略

            boost::uint32_t trunk_len;              // 当前位置到结束的块的个数 X 8 byte
            boost::uint32_t mid;                    // 命令方向和ID

            boost::uint64_t content_length;

        public:
            MmsHead()
                : rep( 0x01 )
                , ver( 0x00 )
                , min_ver( 0x00 )
                , pad( 0x00 )
                , session_id( 0xb00bface )
                , msg_len( 0x00000000 )
                , seal( 0x20534d4d )
                , chunk_count( 0x00000000 )
                , seq( 0x0000 )
                , mbz( 0x0000 )
                , timestamp( 0x0000000000000000 )
                , trunk_len( 0x00000000 )
                , mid( 0x00000000 )
            {}
            
            ~MmsHead() {}

        public:
            void set_size( boost::uint32_t trunk_size )
            {
                trunk_len = trunk_size;
                chunk_count = trunk_len + 2;
                msg_len = chunk_count * 8;
            }

            size_t get_size()
            {
                return 40;
            }

        public:
            void get_content(
                std::ostream & os) const;

            void set_content(
                std::istream & is);

            void get_content(
                std::ostream & os, 
                boost::system::error_code & ec) const;

            void set_content(
                std::istream & is, 
                boost::system::error_code & ec);
        };

        template <typename Archive>
        void serialize(Archive & ar, util::protocol::MmsHead & t)
        {
            ar & t.rep;
            ar & t.ver;
            ar & t.min_ver;
            ar & t.pad;
            ar & t.session_id;
            ar & t.msg_len;
            ar & t.seal;
            ar & t.chunk_count;
            ar & t.seq;
            ar & t.mbz;
            ar & t.timestamp;

            ar & t.trunk_len;
            ar & t.mid;
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MMS_HEAD_H_
