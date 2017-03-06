// Convert.h

/** Convert�����ַ�����ת��
���ڲ�ͬ��ʵ�ַ�ʽ����linux���ܹ�֧���ձ�ı����ʽ����Windows��ֻ֧��
unicode��acp��tacp��utf8��gbk
*/

#ifndef _FRAMEWORK_STRING_CONVERT_H_
#define _FRAMEWORK_STRING_CONVERT_H_

namespace framework
{
    namespace string
    {

        class Convert
        {
        public:
            // ����һ��ת������ָ��Դ�����ʽ��Ŀ������ʽ
            Convert(
                std::string const & to_code, 
                std::string const & from_code);

            ~Convert();

        public:
            // ʹ��ת����ת���ַ�����
            boost::system::error_code convert(
                std::string const & from, 
                std::string & to);

            std::size_t err_pos() const
            {
                return err_pos_;
            }

        private:
            std::string t1; // Ŀ������ʽ
            std::string t2; // Դ�����ʽ
            std::size_t err_pos_;
        };

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_CONVERT_H_
