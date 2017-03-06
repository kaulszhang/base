// Base64.cpp

#include "framework/Framework.h"
#include "framework/string/Base64.h"

namespace framework
{
    namespace string
    {

        static char const tbl_encode[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        static char const tbl_decode[] =
        {
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', 
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', 
            //      !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\076', '\377', '\377', '\377', '\077', 
            //0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
            '\064', '\065', '\066', '\067', '\070', '\071', '\072', '\073', '\074', '\075', '\377', '\377', '\377', '\377', '\377', '\377', 
            //@     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
            '\377', '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007', '\010', '\011', '\012', '\013', '\014', '\015', '\016', 
            //P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _
            '\017', '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027', '\030', '\031', '\377', '\377', '\377', '\377', '\377', 
            //`     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
            '\377', '\032', '\033', '\034', '\035', '\036', '\037', '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047', '\050', 
            //p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~    DEL
            '\051', '\052', '\053', '\054', '\055', '\056', '\057', '\060', '\061', '\062', '\063', '\377', '\377', '\377', '\377', '\377', 
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', 
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', 
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', 
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', 
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', 
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', 
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', 
            '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', '\377', 
        };

        size_t Base64::encode(
            char const * str_src, 
            size_t size_src, 
            char * str_dst, 
            size_t size_dst)
        {
            size_t min_size_dst = 
                (size_src - 1) / 3 * 4 + 4;
            if (size_dst < min_size_dst)
                return min_size_dst;

            // 每次取3个字节，编码成4个字符
            for (; size_src >= 3; str_src += 3, size_src -= 3, str_dst += 4) {
                // 编码成4个字符
                str_dst[0] = tbl_encode[(str_src[0] >> 2) & 0x3f];
                str_dst[1] = tbl_encode[(((str_src[0] << 4) & 0x30) | ((str_src[1] >> 4) & 0x0f))];
                str_dst[2] = tbl_encode[(((str_src[1] << 2) & 0x3C) | ((str_src[2] >> 6) & 0x03))];
                str_dst[3] = tbl_encode[str_src[2] & 0x3f];
            }
            // 编码余下的字节
            if (size_src == 1) {
                str_dst[0] = tbl_encode[(str_src[0] >> 2) & 0x3f];
                str_dst[1] = tbl_encode[(str_src[0] << 4) & 0x30];
                str_dst[2] = '=';
                str_dst[3] = '=';
            } else if (size_src == 2) {
                str_dst[0] = tbl_encode[(str_src[0] >> 2) & 0x3f];
                str_dst[1] = tbl_encode[(((str_src[0] << 4) & 0x30) | ((str_src[1] >> 4) & 0x0f))];
                str_dst[2] = tbl_encode[(str_src[1] << 2) & 0x3C];
                str_dst[3] = '=';
            }

            return min_size_dst;
        }

        size_t Base64::decode(
            char const * str_src, 
            size_t size_src, 
            char * str_dst, 
            size_t size_dst)
        {
            //assert(size_src % 4 == 0);

            if (size_src % 4 != 0)
                return 0;

            size_t min_size_dst = 
                size_src / 4 * 3;
            for (char const * p_src = str_src + size_src - 1; 
                *p_src == '='; 
                --p_src, --min_size_dst);
            if (size_dst < min_size_dst)
                return min_size_dst;

            // 取4个字符，解码到一个长整数，再经过移位得到3个字节
            for (; size_src > 4; str_src += 4, size_src -= 4, str_dst += 3) {
                boost::uint32_t three_bytes = 0;   // 解码用到的长整数
                if (tbl_decode[( unsigned char )str_src[0]] == '\377' 
                    || tbl_decode[( unsigned char )str_src[0]] == '\377'
                    || tbl_decode[( unsigned char )str_src[2]] == '\377'
                    || tbl_decode[( unsigned char )str_src[3]] == '\377')
                    return 0;
                three_bytes = tbl_decode[( unsigned char )str_src[0]] << 18;
                three_bytes |= tbl_decode[( unsigned char )str_src[1]] << 12;
                three_bytes |= tbl_decode[( unsigned char )str_src[2]] << 6;
                three_bytes |= tbl_decode[( unsigned char )str_src[3]];

                str_dst[0] = (char)((three_bytes & 0x00ff0000) >> 16);
                str_dst[1] = (char)((three_bytes & 0x0000ff00) >> 8);
                str_dst[2] = (char)((three_bytes & 0x000000ff));
            }

            size_t i = 0;
            boost::uint32_t three_bytes = 0;   // 解码用到的长整数
            for (; i < 4 && str_src[i] != '='; ++i) {
                if (tbl_decode[( unsigned char )str_src[i]] == '\377')
                    return 0;
                three_bytes = (three_bytes << 6) | tbl_decode[( unsigned char )str_src[i]];
            }
            three_bytes >>= ((4 - i) * 2);
            for (----i; i != size_t(-1); --i) {
                str_dst[i] = (char)three_bytes;
                three_bytes >>= 8;
            }

            return min_size_dst;
        }

        std::string Base64::encode(
            char const * str, 
            size_t size)
        {
            std::string result;
            result.resize(encode(str, size, NULL, 0));
            if (!result.empty())
                result.resize(encode(str, size, &result.at(0), result.size()));
            return result;
        }

        std::string Base64::decode(
            char const * str, 
            size_t size)
        {
            std::string result;
            result.resize(decode(str, size, NULL, 0));
            if (!result.empty())
                result.resize(decode(str, size, &result.at(0), result.size()));
            return result;
        }

        std::string Base64::decode(
            char const * str)
        {
            return decode(str, strlen(str));
        }

    } // namespace string
} // namespace framework
