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

            /// ������д����������ͱ���
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

            /// ������д����׼���ַ���
            void save(
                std::string const & t)
            {
                os_ << t;
            }

            using StreamOArchive<ConsoleOArchive>::save;

            /// ������д�����飨�Ż���
            void save_array(
                framework::container::Array<char> const & a)
            {
                save_binary((char const *)a.address(), sizeof(char) * a.count());
            }

            /// �ж�ĳ�������Ƿ�����Ż���������л�
            /// ֻ�л��������ܹ�ֱ�����л�����
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
