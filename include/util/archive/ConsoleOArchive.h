// ConsoleOArchive.h

#ifndef _UTIL_ARCHIVE_CONSOLE_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_CONSOLE_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"

#include <boost/type_traits/is_same.hpp>

#include <iostream>
#include <string>
#include <ostream>

namespace util
{
    namespace archive
    {

        class ConsoleOArchive
            : public StreamOArchive<ConsoleOArchive>
        {
            friend class StreamOArchive<ConsoleOArchive>;

        public:
            ConsoleOArchive(
                std::ostream & os = std::cout)
                : StreamOArchive<ConsoleOArchive>(*os.rdbuf())
                , sub_just_end_(true)
                , os_(os)
            {
            }

            /// 向流中写入参数化类型变量
            template <typename T>
            void save(
                T const & t)
            {
                os_ << t;
            }

            void save(
                unsigned char const & t)
            {
                unsigned short t1 = t;
                os_ << t1;
            }

            /// 向流中写入变标准库字符串
            void save(
                std::string const & t)
            {
                os_ << t;
            }

            using StreamOArchive<ConsoleOArchive>::save;

            /// 向流中写入数组（优化）
            void save_array(
                framework::container::Array<char> const & a)
            {
                save_binary((char const *)a.address(), sizeof(char) * a.count());
            }

            /// 判断某个类型是否可以优化数组的序列化
            /// 只有基本类型能够直接序列化数组
            template<class T>
            struct use_array_optimization
                : boost::is_same<T, char>
            {
            };

            void save_start(
                std::string const & name)
            {
                sub_just_end_ = false;
                os_ << ident_ << name << ": ";
            }

            void save_end(
                std::string const & name)
            {
                if (!sub_just_end_)
                    os_ << std::endl;
                sub_just_end_ = true;
            }

            void sub_start()
            {
                os_ << std::endl;
                sub_just_end_ = false;
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
            std::ostream & os_;
        };

    } // namespace archive
} // namespace util

SERIALIZATION_USE_ARRAY_OPTIMIZATION(util::archive::ConsoleOArchive);

#endif // _UTIL_ARCHIVE_CONSOLE_O_ARCHIVE_H_
