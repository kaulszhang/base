// Base64.cpp

#include "util/Util.h"
#include "util/protocol/pptv/Base64.h"

#include <framework/string/Base64.h>

namespace util
{
    namespace protocol
    {

        namespace pptv
        {

            std::string base64_encode(
                std::string const & str, 
                std::string const & key)
            {
                char const * PPL_KEY = key.c_str();
                size_t const PPL_KEY_LENGTH = key.size();
                std::string result;
                result.resize(str.size());
                for (size_t i = 0; i < str.size(); ++i)
                {
                    size_t keyIndex = i % PPL_KEY_LENGTH;
                    result[i] = str[i] + PPL_KEY[keyIndex];
                }

                return framework::string::Base64::encode(result);
            }

            std::string base64_decode(
                std::string const & str, 
                std::string const & key)
            {
                char const * PPL_KEY = key.c_str();
                size_t const PPL_KEY_LENGTH = key.size();

                std::string str2 = framework::string::Base64::decode(str);
                std::string result;
                result.resize(str2.size());
                for (size_t i = 0; i < str2.size(); ++i)
                {
                    size_t keyIndex = i % PPL_KEY_LENGTH;
                    result[i] = str2[i] - PPL_KEY[keyIndex];
                }
                return result;
            }

        }

    } // namespace protocol
} // namespace util

