// StringToken.h

#ifndef _FRAMEWORK_STRING_STRING_TOKEN_H
#define _FRAMEWORK_STRING_STRING_TOKEN_H

namespace framework
{
    namespace string
    {

        class StringToken
        {
        public:
            StringToken(
                std::string const & str, 
                std::string const & delim, 
                bool skip_empty = false, 
                std::string::size_type beg = 0, 
                std::string::size_type end = std::string::npos);

        public:
            std::string next_token();

            void next_token(
                std::string & token);

            boost::system::error_code next_token(
                std::string & token, 
                boost::system::error_code & ec);

            boost::system::error_code next_token(
                boost::system::error_code & ec);

            std::string remain() const;

        private:
            std::string const & str_;
            std::string delim_;
            bool skip_empty_;
            std::string::size_type pos_;
            std::string::size_type end_;
        };

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_STRING_TOKEN_H
