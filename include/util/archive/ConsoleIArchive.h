// ConsoleIArchive.h

#ifndef _UTIL_ARCHIVE_CONSOLE_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_CONSOLE_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"

#include <boost/type_traits/is_same.hpp>

#include <string>
#include <iostream>
#include <limits>

#undef max

namespace util
{
    namespace archive
    {

        class ConsoleIArchive
            : public StreamIArchive<ConsoleIArchive>
        {
            friend class StreamIArchive<ConsoleIArchive>;
            friend struct LoadAccess;

        public:
            ConsoleIArchive(
                std::istream & is = std::cin, 
                std::ostream & os = std::cout)
                : StreamIArchive<ConsoleIArchive>(*is.rdbuf())
                , sub_just_end_(true)
                , is_(is)
                , os_(os)
            {
            }

        public:
            /// 从流中读出变量
            template<class T>
            void load(
                T & t)
            {
                is_ >> t;
                if (is_.fail()) {
                    os_ << "wrong!!!";
                    state_ = 1;
                    is_.clear();
                    is_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            }

            void load(
                unsigned char & t)
            {
                unsigned short t1;
                is_ >> t1;
                if (is_.fail()) {
                    os_ << "wrong!!!";
                    state_ = 1;
                    is_.clear();
                    is_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                if (t1 > 255)
                    state_ = 1;
                else
                    t = (unsigned char)t1;
            }

            /// 从流中读出变标准库字符串
            void load(
                std::string & t)
            {
                is_ >> t;
                if (is_.fail()) {
                    os_ << "wrong!!!";
                    state_ = 1;
                    is_.clear();
                    is_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            }

            using StreamIArchive<ConsoleIArchive>::load;

            /// 从流中读出数组（优化）
            void load_array(
                util::container::Array<char> & a)
            {
                load_binary((char *)a.address(), sizeof(char) * a.count());
            }

            /// 判断某个类型是否可以优化数组的序列化
            /// 只有基本类型能够直接序列化数组
            template<class T>
            struct use_array_optimization
                : boost::is_same<T, char>
            {
            };

            void load_start(
                std::string const & name)
            {
                os_ << ident_ << name << ": ";
            }

            void load_end(
                std::string const & name)
            {
                sub_just_end_ = true;
            }

            void sub_start()
            {
                os_ << std::endl;
                sub_just_end_ = false;
                os_ << std::endl;
                ident_ += "  ";
            }

            void sub_end()
            {
                if (!sub_just_end_)
                    os_ << std::endl;
                sub_just_end_ = true;
                ident_.erase(ident_.size() - 2);
            }

        private:
            std::string ident_;
            bool sub_just_end_;
            std::istream & is_;
            std::ostream & os_;
        };


    } // namespace archive
} // namespace util

SERIALIZATION_USE_ARRAY_OPTIMIZATION(Util::Serialize::ConsoleIArchive);

#endif // _UTIL_ARCHIVE_CONSOLE_I_ARCHIVE_H_
