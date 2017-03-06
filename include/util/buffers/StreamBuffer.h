// StreamBuffer.h

#ifndef _UTIL_BUFFERS_STREAM_BUFFER_H_
#define _UTIL_BUFFERS_STREAM_BUFFER_H_

#include <streambuf>
#include <stdexcept>
#include <limits>
#include <algorithm> 

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>, 
            typename _Alloc = std::allocator<_Elem>
        >
        class StreamBuffer
            : public std::basic_streambuf<_Elem, _Traits>
        {
        public:
            typedef typename std::basic_streambuf<_Elem, _Traits>::char_type char_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::int_type int_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::traits_type traits_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::pos_type pos_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::off_type off_type;

            typedef boost::asio::const_buffers_1 const_buffers_type;
            typedef boost::asio::mutable_buffers_1 mutable_buffers_type;

        public:
            StreamBuffer(
                std::size_t max_size = (std::numeric_limits<std::size_t>::max)(),
                _Alloc const & allocator = _Alloc())
                : max_size_(max_size)
                , buffer_(allocator)
                , pos_(0)
            {
                std::size_t pend = (std::min<std::size_t>)(max_size_, buffer_delta);
                buffer_.resize((std::max<std::size_t>)(pend, 1));
                this->setg(&buffer_[0], &buffer_[0], &buffer_[0]);
                this->setp(&buffer_[0], &buffer_[0] + pend);
            }

            StreamBuffer(
                StreamBuffer const & r)
                : max_size_(r.max_size_)
                , buffer_(r.buffer_)
                , pos_(r.pos_)
            {

                std::size_t gnext = r.gptr() - &r.buffer_[0];
                std::size_t gend = r.egptr() - &r.buffer_[0];
                std::size_t pnext = r.pptr() - &r.buffer_[0];
                std::size_t pend = r.epptr() - &r.buffer_[0];
                // Update stream positions.
                this->setg(&buffer_[0], &buffer_[0] + gnext, &buffer_[0] + gend);
                this->setp(&buffer_[0] + pnext, &buffer_[0] + pend);
            }

        public:
            StreamBuffer & operator=(
                StreamBuffer const & r)
            {
                max_size_ = r.max_size_;
                buffer_ = r.buffer_;
                pos_ = r.pos_;

                std::size_t gnext = r.gptr() - &r.buffer_[0];
                std::size_t gend = r.egptr() - &r.buffer_[0];
                std::size_t pnext = r.pptr() - &r.buffer_[0];
                std::size_t pend = r.epptr() - &r.buffer_[0];
                // Update stream positions.
                this->setg(&buffer_[0], &buffer_[0] + gnext, &buffer_[0] + gend);
                this->setp(&buffer_[0] + pnext, &buffer_[0] + pend);

                return *this;
            }

        public:
            std::size_t size()
            {
                return this->pptr() - this->gptr();
            }

            /// Return the maximum size of the buffer.
            std::size_t max_size() const
            {
                return max_size_;
            }

            /**
            返回读缓存的数据

            并不移动读指针。
            */
            const_buffers_type data(
                std::size_t n = std::size_t(-1)) const
            {
                if (n > (size_t)(this->pptr() - this->gptr()))
                    n = this->pptr() - this->gptr();
                return const_buffers_type(this->gptr(), n);
            }

            /**
            在写缓存中准备一段空间

            并不移动写指针。
            */
            mutable_buffers_type prepare(
                std::size_t n)
            {
                reserve(n);
                return mutable_buffers_type(this->pptr(), n);
            }

            /**
            将写缓存的数据提交的读缓存

            追加从写指针开始的 @c n 字节数据到读缓存，写指针向前移动 @c n 字节。
            需要在外部写入实际数据，然后提交
             */
            void commit(
                std::size_t n)
            {
                if (n > (size_t)(this->epptr() - this->pptr()))
                    n = this->epptr() - this->pptr();
                this->pbump(static_cast<int>(n));
                this->setg(this->eback(), this->gptr(), this->pptr());
            }

            /**
            从读缓存移除数据

            读指针向前移动 @c n 字节。
            需要在外部读出实际数据，然后移除相应数据
            */
            void consume(
                std::size_t n)
            {
                if (n > (size_t)(this->pptr() - this->gptr()))
                    n = this->pptr() - this->gptr();
                this->gbump(static_cast<int>(n));
                n = this->pptr() - this->gptr();
                this->setp(this->gptr(), this->epptr());
                this->pbump(static_cast<int>(n));
            }

            void reset()
            {
                std::size_t pend = buffer_.size();
                this->setg(&buffer_[0], &buffer_[0], &buffer_[0]);
                this->setp(&buffer_[0], &buffer_[0] + pend);
                pos_ = 0;
            }

        protected:
            enum { buffer_delta = 128 };

            virtual int_type underflow()
            {
                if (this->gptr() < this->pptr()) {
                    this->setg(&buffer_[0], this->gptr(), this->pptr());
                    return traits_type::to_int_type(*this->gptr());
                }
                return traits_type::eof();
            }

            virtual int_type overflow(int_type c)
            {
                if (!traits_type::eq_int_type(c, traits_type::eof()))
                {
                    if (this->pptr() == this->epptr())
                    {
                        std::size_t buffer_size = this->pptr() - this->gptr();
                        if (buffer_size < max_size_ && max_size_ - buffer_size < buffer_delta)
                        {
                            reserve(max_size_ - buffer_size);
                        }
                        else
                        {
                            reserve(buffer_delta);
                        }
                    }

                    *this->pptr() = traits_type::to_char_type(c);
                    this->pbump(1);
                    return c;
                }

                return traits_type::not_eof(c);
            }

            virtual pos_type seekoff(
                off_type off, 
                std::ios_base::seekdir dir,
                std::ios_base::openmode mode)
            {
                if (dir == std::ios_base::cur) {
                    pos_type pos = (mode == std::ios_base::in ? this->gptr() : this->pptr()) - &buffer_[0] + pos_;
                    if (off == 0) {
                        return pos;
                    }
                    pos += off;
                    return seekpos(pos, mode);
                } else if (dir == std::ios_base::beg) {
                    return seekpos(off, mode);
                } else if (dir == std::ios_base::end) {
                    assert(off <= 0);
                    pos_type pos = this->pptr() - &buffer_[0] + pos_;
                    return seekpos(pos + off, mode);
                } else {
                    return pos_type(-1);
                }
            }

            virtual pos_type seekpos(
                pos_type position, 
                std::ios_base::openmode mode)
            {
                assert(position != pos_type(-1));
                if (position < pos_) {
                    return pos_type(-1);
                }
                position -= pos_;
                if (mode == std::ios_base::in) {
                    if (position <= this->pptr() - &buffer_[0]) {
                        this->setg(this->eback(), this->eback() + position, this->pptr());
                    } else {
                        return pos_type(-1);
                    }
                } else if (mode == std::ios_base::out) {
                    if (position >= this->gptr() - &buffer_[0] && position <= pos_type(buffer_.size())) {
                        this->setp(this->gptr(), this->epptr());
                        this->pbump(&buffer_[0] + position - this->gptr());
                    } else {
                        return pos_type(-1);
                    }
                } else {
                    if ((size_t)position <= buffer_.size()) {
                        this->setg(&buffer_[0], &buffer_[0] + position, &buffer_[0] + position);
                        this->setp(&buffer_[0] + position, &buffer_[0] + buffer_.size());
                    } else {
                        return pos_type(-1);
                    }
                }
                return position + pos_;
            }

            void reserve(std::size_t n)
            {
                // Get current stream positions as offsets.
                std::size_t gnext = this->gptr() - &buffer_[0];
                std::size_t gend = this->egptr() - &buffer_[0];
                std::size_t pnext = this->pptr() - &buffer_[0];
                std::size_t pend = this->epptr() - &buffer_[0];

                // Check if there is already enough space in the put area.
                if (n <= pend - pnext)
                {
                    return;
                }

                // Shift existing contents of get area to start of buffer.
                if (gnext > 0)
                {
                    std::rotate(&buffer_[0], &buffer_[0] + gnext, &buffer_[0] + pend);
                    gend -= gnext;
                    pnext -= gnext;
                    pos_ += gnext;
                }

                // Ensure buffer is large enough to hold at least the specified size.
                if (n > pend - pnext)
                {
                    if (n <= max_size_ && pnext <= max_size_ - n)
                    {
                        buffer_.resize((std::max<std::size_t>)(pnext + n, 1));
                    }
                    else
                    {
                        throw std::length_error("util::buffers::StreamBuffer too long");
                    }
                }

                // Update stream positions.
                this->setg(&buffer_[0], &buffer_[0], &buffer_[0] + gend);
                this->setp(&buffer_[0] + pnext, &buffer_[0] + pnext + n);
            }

        private:
            std::size_t max_size_;
            std::vector<_Elem, _Alloc> buffer_;
            pos_type pos_;
        };

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_STREAM_BUFFER_H_
