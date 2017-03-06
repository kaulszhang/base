// Convert.h

/** Convert用于字符编码转换
由于不同的实现方式，在linux下能够支持普遍的编码格式，在Windows下只支持
unicode、acp、tacp、utf8、gbk
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
            // 构造一定转换器，指定源编码格式和目标编码格式
            Convert(
                std::string const & to_code, 
                std::string const & from_code);

            ~Convert();

        public:
            // 使用转换器转换字符编码
            boost::system::error_code convert(
                std::string const & from, 
                std::string & to);

            std::size_t err_pos() const
            {
                return err_pos_;
            }

        private:
            std::string t1; // 目标编码格式
            std::string t2; // 源编码格式
            std::size_t err_pos_;
        };

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_CONVERT_H_
