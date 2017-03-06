// Base16.cpp

#include "framework/Framework.h"
#include "framework/string/Base16.h"

namespace framework
{
    namespace string
    {

        std::string Base16::encode(
            std::string const & data, 
            bool up_cast)
        {
            char const * hex_chr = up_cast ? "0123456789ABCDEF" : "0123456789abcdef";
            std::string hex;
            for (unsigned int i = 0; i < data.length(); i++) {
                unsigned char d = (unsigned char)data[i];
                hex.append(1, hex_chr[d >> 4]);
                hex.append(1, hex_chr[d & 0x0F]);
            }
            return hex;
        }

        std::string Base16::decode(
            std::string const & hex)
        {
            std::string data;
            if (hex.length() % 2) {
                return "E1";
            }
            for (unsigned int i = 0; i < hex.length() / 2; i++) {
                char h = hex[i * 2];
                char l = hex[i * 2 + 1];
                if (h >= '0' && h <= '9')
                    h -= '0';
                else if (h >= 'A' && h <= 'F')
                    h -= ('A' - 10);
                else if (h >= 'a' && h <= 'f')
                    h -= ('a' - 10);
                else {
                    return "E2";
                }
                if (l >= '0' && l <= '9')
                    l -= '0';
                else if (l >= 'A' && l <= 'F')
                    l -= ('A' - 10);
                else if (l >= 'a' && l <= 'f')
                    l -= ('a' - 10);
                else {
                    return "E2";
                }
                data.append(1, char((h << 4) + l));
            }
            return data;
        }

    } // namespace string
} // namespace framework
