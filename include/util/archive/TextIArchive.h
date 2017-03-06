// TextIArchive.h

#ifndef _UTIL_ARCHIVE_TEXT_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_TEXT_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"

#include <istream>
#include <string>

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class TextIArchive
            : public StreamIArchive<TextIArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamIArchive<TextIArchive<_Elem, _Traits>, _Elem, _Traits>;

        public:
            TextIArchive(
                std::basic_istream<_Elem, _Traits> & is)
                : StreamIArchive<TextIArchive<_Elem, _Traits>, _Elem, _Traits>(*is.rdbuf())
                , is_(is)
                , local_is_(false)
            {
            }

            TextIArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamIArchive<TextIArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
                , is_(*new std::basic_istream<_Elem, _Traits>(&buf))
                , local_is_(true)
            {
            }

            ~TextIArchive()
            {
                if (local_is_)
                    delete &is_;
            }

        public:
            /// 从流中读出变量
            template<class T>
            void load(
                T & t)
            {
                if (this->state()) return;
                is_ >> t;
                if (is_.fail())
                    this->state(1);
                is_.get(); // 跳过空格
            }

            using StreamIArchive<TextIArchive, _Elem, _Traits>::load;

        private:
            std::basic_istream<_Elem, _Traits> & is_;
            bool local_is_;
        };

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_TEXT_I_ARCHIVE_H_
