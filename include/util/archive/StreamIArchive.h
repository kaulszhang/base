// StreamIArchive.h

#ifndef _UTIL_ARCHIVE_STREAM_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_STREAM_I_ARCHIVE_H_

#include "util/archive/BasicIArchive.h"

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
        class StreamIArchive
            : public BasicIArchive<Archive>
        {
            friend class BasicIArchive<Archive>;
        public:
            typedef _Elem char_type;

            typedef _Traits traits_type;

            typedef typename _Traits::pos_type pos_type;

            typedef typename _Traits::off_type off_type;

            StreamIArchive(
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

            StreamIArchive & seekg(
                off_type off, 
                std::ios_base::seekdir dir)
            {
                if (!this->state() 
                    && buf_->pubseekoff(off, dir, std::ios_base::in) == pos_type(-1))
                        this->state(3);
                return (*this);
            }

            StreamIArchive & seekg(
                pos_type pos, 
                std::ios_base::seekdir dir)
            {
                if (!this->state() 
                    && buf_->pubseekpos(pos, std::ios_base::in) == pos_type(-1))
                    this->state(3);
                return (*this);
            }

            pos_type tellg() const
            {
                if (!this->state())
                    return (buf_->pubseekoff(0, std::ios_base::cur, std::ios_base::in));
                else
                    return (pos_type(-1));
            }

        protected:
            void load_binary(
                _Elem * p, 
                std::size_t n)
            {
                if (this->state())
                    return;
                if (buf_->sgetn(p, (std::streamsize)n) 
                    != (std::streamsize)n)
                    this->state(1);
            }

        protected:
            std::basic_streambuf<_Elem, _Traits> * buf_;
        };

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_STREAM_I_ARCHIVE_H_
