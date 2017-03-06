// Base16.h

/** �ṩ�����ݽ���16���Ʊ���ͽ���ĺ���
    
    ���ֽ�Ϊ��λ��ÿ���ֽ�������16�����ַ���ʾ���硰0xA5����ʾΪ ��A5��
*/
#ifndef _FRAMEWORK_STRING_BASE16_H_
#define _FRAMEWORK_STRING_BASE16_H_

namespace framework
{
    namespace string
    {
        struct Base16
        {
            // ����
            static std::string encode(
                std::string const & data, 
                bool up_cast = true);

            // ����
            static std::string decode(
                std::string const & hex);
        };
    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_BASE16_H_
