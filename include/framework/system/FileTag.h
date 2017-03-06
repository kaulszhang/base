// FileTag.h

#ifndef _FRAMEWORK_SYSTEM_FILE_TAG_H_ 
#define _FRAMEWORK_SYSTEM_FILE_TAG_H_

#include <boost/preprocessor/stringize.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/regex.hpp>

#include <fstream>

#define FILE_TAG_VALUE(type, name, value) \
    "!" \
        BOOST_PP_STRINGIZE(name) "|" BOOST_PP_STRINGIZE(type) "|tag" \
        value "\000                      !" \
    + sizeof(BOOST_PP_STRINGIZE(name)) + sizeof(BOOST_PP_STRINGIZE(type)) + 4

#define DEFINE_FILE_TAG_NAME_TYPE(type, name, value) \
static char const * name ## _ ## type ## _string() \
{ \
    return FILE_TAG_VALUE(type, name, value); \
}

#define FILE_TAG(type, name) name ## _ ## type ## _string()

#define DEFINE_FILE_TAG_NAME(type, name, value) \
static char const * name ## _string() \
{ \
    return FILE_TAG_VALUE(type, name, value); \
}

#define FILE_TAG_NAME(name) name ## _string()

#define DEFINE_FILE_TAG_TYPE(type, name, value) \
static char const * name ## _string() \
{ \
    return FILE_TAG_VALUE(type, name, value); \
}

#define FILE_TAG_TYPE(type) type ## _string()

namespace framework
{
    namespace system
    {

        class FileFinder
        {
        public:
            FileFinder(
                std::string const & file);

            bool find(
                std::string const & regex, 
                std::string & result);

            void read(
                std::string & result);

            void write(
                std::string const & result);

            bool fail() const;

            void close();

        private:
            std::fstream fs_;
            boost::asio::streambuf buf_;
            size_t pos_;
        };

        class FileTag
        { 
        public:
            FileTag(
                std::string const & file, 
                std::string const & tag);

        public:
            boost::system::error_code get(
                std::string const & item, 
                std::string & value);

            boost::system::error_code set(
                std::string const & item, 
                std::string const & value);

            boost::system::error_code get_all(
                std::map<std::string, std::string> & values);

            boost::system::error_code set_all(
                std::map<std::string, std::string> & values);

        private:
            std::string const file_;
            std::string const tag_;
        };

    } // namespace system
} // namespace boost

#endif // _FRAMEWORK_SYSTEM_FILE_TAG_H_
