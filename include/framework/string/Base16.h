// Base16.h

/** 提供将数据进行16进制编码和解码的函数
    
    以字节为单位，每个字节用两个16进制字符表示，如“0xA5”表示为 “A5”
*/
#ifndef _FRAMEWORK_STRING_BASE16_H_
#define _FRAMEWORK_STRING_BASE16_H_

namespace framework
{
    namespace string
    {
        struct Base16
        {
            // 编码
            static std::string encode(
                std::string const & data, 
                bool up_cast = true);

            // 解码
            static std::string decode(
                std::string const & hex);
        };
    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_BASE16_H_
