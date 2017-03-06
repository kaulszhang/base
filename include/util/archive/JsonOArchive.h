// JsonOArchive.h

#ifndef _UTIL_ARCHIVE_JSON_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_JSON_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"

#include <boost/type_traits/is_same.hpp>

#include <iostream>
#include <string>
#include <ostream>

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class JsonOArchive
            : public StreamOArchive<JsonOArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamOArchive<JsonOArchive<_Elem, _Traits>, _Elem, _Traits>;

        public:
            JsonOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : StreamOArchive<JsonOArchive<_Elem, _Traits>, _Elem, _Traits>(*os.rdbuf())
                , os_(os)
                , local_os_(false)
            {
            }

            JsonOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamOArchive<JsonOArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
                , os_(*new std::basic_ostream<_Elem, _Traits>(&buf))
                , local_os_(true)
                , sub_just_end_(true)
            {
            }

            ~JsonOArchive()
            {
                if (local_os_)
                    delete &os_;
            }

        public:
            /// 向流中写入参数化类型变量
            template <typename T>
            void save(
                T const & t)
            {
                if (splits.back() == 'c') {
                    return;
                }
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
                os_ << "'" << t << "'";
            }

            using StreamOArchive<JsonOArchive<_Elem, _Traits>, _Elem, _Traits>::save;

            void save_start(
                std::string const & name)
            {
                assert(splits.size() >= 2); // named object can only occur in object or array (count, item)
                if (name == "count" && splits.back() == '{') {
                    splits.pop_back();
                    splits.back() = ']';
                    splits.push_back('c');
                } else {
                    os_ << splits.back() << std::endl;
                    os_ << ident_;
                    if (name != "item" || splits[splits.size() - 2] != ']') {
                        os_ << "\"" << name << "\":";
                    }
                }
            }

            void save_end(
                std::string const & name)
            {
                if (splits.back() == 'c') {
                    splits.back() = '[';
                } else {
                    splits.back() = ',';
                }
            }

            void sub_start()
            {
                splits.push_back('}');
                splits.push_back('{');
                ident_ += "  ";
                sub_just_end_ = true;
            }

            void sub_end()
            {
                os_ << std::endl;
                splits.pop_back();
                ident_.erase(ident_.size() - 2);
                os_ << ident_ << splits.back();
                splits.pop_back();
                sub_just_end_ = false;
            }

        private:
            std::basic_ostream<_Elem, _Traits> & os_;
            bool local_os_;
            std::vector<char> splits;
            std::string ident_;
            bool sub_just_end_;
        };

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_JSON_O_ARCHIVE_H_
