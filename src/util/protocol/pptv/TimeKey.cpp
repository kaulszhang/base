// TimeKey.cpp

#include "util/Util.h"
#include "util/protocol/pptv/TimeKey.h"

#include <framework/system/BytesOrder.h>
using namespace framework::system;

#include <string.h>
#include <stdlib.h>

namespace util
{
    namespace protocol
    {

        namespace pptv
        {

            static unsigned int GetkeyFromstr(unsigned char* str, size_t len)
            {
                size_t i=0;
                union tagkey
                {
                    unsigned char ch[4];
                    unsigned int key;
                }tmp_key;
                memset(&tmp_key,0,sizeof(tmp_key));
                for(i=0;i<len;i++)
                {
                    tmp_key.ch[i%4] ^= (unsigned char)str[i];
                }
                //return tmp_key.key;
                return BytesOrder::little_endian_to_host_long(tmp_key.key);
            }

            static void TGetKey(
                const unsigned int *k0, 
                unsigned int *k1,
                unsigned int *k2, 
                unsigned int *k3)
            {
                *k1 = *k0<<8|*k0>>24;
                *k2 = *k0<<16|*k0>>16;
                *k3 = *k0<<24|*k0>>8;
            }

            static const size_t ENCRYPT_ROUNDS = 32; // at least 32
            static const boost::uint32_t DELTA = 0x9E3779B9;
            static const size_t BLOCK_SIZE = (sizeof(unsigned int) << 1);
            static const size_t BLOCK_SIZE_TWICE = ((sizeof(unsigned int) << 1) << 1);
            static const size_t BLOCK_SIZE_HALF = ((sizeof(unsigned int) << 1) >> 1);

            static void TEncrypt(
                unsigned char *buffer, 
                unsigned int buf_size, 
                char* key, 
                size_t len)
            {
                size_t i;
                unsigned int k0 = GetkeyFromstr((unsigned char *)key, len), k1, k2, k3;
                TGetKey(&k0, &k1, &k2, &k3);
                for (i = 0; i + BLOCK_SIZE_TWICE <= buf_size; i += BLOCK_SIZE_TWICE)
                {
                    //unsigned int *v = (unsigned int*) (buffer + i);
                    unsigned int v[2];
                    memcpy(v,buffer + i,sizeof(int)*2);
                    unsigned int v0 = BytesOrder::little_endian_to_host_long(v[0]);
                    unsigned int v1 = BytesOrder::little_endian_to_host_long(v[1]);
                    unsigned int sum = 0;
                    size_t j;
                    for (j = 0; j < ENCRYPT_ROUNDS; j++)
                    {
                        sum += DELTA;
                        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
                        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
                    }
                    v[0] = BytesOrder::host_to_little_endian_long(v0);
                    v[1] = BytesOrder::host_to_little_endian_long(v1);
                    memcpy(buffer + i,v,sizeof(int)*2);
                }
            }

            static void Time2Str(
                unsigned int timet, 
                unsigned char str[], 
                unsigned int len)
            {
                for(unsigned int i=0; i<len && i < 8;i++)
                {
                    str[i] = (unsigned char)((timet >> (28 - i % 8 * 4)) & 0xF);
                    str[i] += (unsigned char)(str[i] > 9 ? 'a'-(unsigned char)10 : '0');
                }
            }

            static int Str2Hex(
                unsigned char buffer[], 
                unsigned int buf_size, 
                unsigned char hexstr[], 
                unsigned int hs_size)
            {
                if (hs_size < 2*buf_size+1) return 0;

                for (unsigned int i = 0; i < buf_size; i++)
                {
                    hexstr[2 * i] = (unsigned char)(buffer[i] & 0xF);
                    hexstr[2 * i + 1] = (unsigned char)((buffer[i] >> 4) & 0xF);
                }
                for (unsigned int i = 0; i < 2 * buf_size; i++)
                {
                    hexstr[i] += (unsigned char)(hexstr[i] > 9 ? 'a' - (unsigned char)10 : '0');
                }
                hexstr[2*buf_size] = (unsigned char)0;
                return 1;
            }

            std::string gen_key_from_time(
                unsigned int time_now)
            {
                unsigned char bytes[16] = {0};
                std::string keypwd = "qqqqqww";
                char key[16] = {0};
                memcpy(key, keypwd.c_str(), keypwd.length());

                unsigned char result[33];

                Time2Str(time_now, bytes, 16);

                for (int i = 0; i < 16; i++)
                {
                    if (bytes[i] == 0)
                    {
                        bytes[i] = (unsigned char)(rand() % 256);
                    }
                }

                TEncrypt(bytes, 16, key, 16);

                Str2Hex(bytes, 16, result, 33);

                std::string result_str((const char*)result);

                return result_str;
            }

        }

    } // namespace protocol
} // namespace util

