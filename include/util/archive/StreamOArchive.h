// StreamOArchive.h

#ifndef _UTIL_ARCHIVE_STREAM_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_STREAM_O_ARCHIVE_H_

#include "util/archive/BasicOArchive.h"

#include <streambuf>

namespace util
{
    namespace archive
    {

        template <
            typename Archive, 
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class StreamOArchive
            : public BasicOArchive<Archive>
        {
            friend class BasicOArchive<Archive>;
        public:
            typedef _Elem char_type;

            typedef _Traits traits_type;

            typedef typename _Traits::pos_type pos_type;

            typedef typename _Traits::off_type off_type;

            StreamOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : buf_(&buf)
            {
            }

            std::basic_streambuf<_Elem, _Traits> * rdbuf() const
            {
                return buf_;
            }

            std::basic_streambuf<_Elem, _Traits> & streambuf() const
            {
                return *buf_;
            }

            std::basic_streambuf<_Elem, _Traits> * rdbuf(
                std::basic_streambuf<_Elem, _Traits> * b)
            {
                std::basic_streambuf<_Elem, _Traits> * tmp = buf_;
                buf_ = b;
                return tmp;
            }

            StreamOArchive & seekp(
                off_type off, 
                std::ios_base::seekdir dir)
            {
                if (!this->state() 
                    && buf_->pubseekoff(off, dir, std::ios_base::out) == pos_type(-1))
                    this->state(3);
                return (*this);
            }

            StreamOArchive & seekp(
                pos_type pos)
            {
                if (!this->state() 
                    && buf_->pubseekpos(pos, std::ios_base::out) == pos_type(-1))
                    this->state(3);
                return (*this);
            }

            pos_type tellp() const
            {
                if (!this->state())
                    return (buf_->pubseekoff(0, std::ios_base::cur, std::ios_base::out));
                else
                    return (pos_type(-1));
            }

        protected:
            void save_binary(
                _Elem const * p, 
                std::size_t n)
            {
                if (this->state()) return;
                if (buf_->sputn(p, (std::streamsize)n) 
                    != (std::streamsize)n)
                    this->state(1);
            }

        protected:
            std::basic_streambuf<_Elem, _Traits> * buf_;
        };

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_STREAM_O_ARCHIVE_H_
