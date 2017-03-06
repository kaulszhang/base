// ArchiveBuffer.h

#ifndef _UTIL_ARCHIVE_ARCHIVE_BUFFER_H_
#define _UTIL_ARCHIVE_ARCHIVE_BUFFER_H_

#include <streambuf>
#include <stdexcept>

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class ArchiveBuffer
            : public std::basic_streambuf<_Elem, _Traits>
        {
        public:
            typedef typename std::basic_streambuf<_Elem, _Traits>::char_type char_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::int_type int_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::traits_type traits_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::pos_type pos_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::off_type off_type;

            ArchiveBuffer(
                char_type * buf, 
                std::size_t buf_size, 
                std::size_t data_size = 0)
                : buf_(buf)
                , buf_size_(buf_size)
            {
                this->setg(buf_, buf_, buf_ + data_size);
                this->setp(buf_ + data_size, buf_ + buf_size_);
            }

            ArchiveBuffer(
                boost::asio::const_buffer const & buf)
                : buf_((char_type *)boost::asio::buffer_cast<char_type const *>(buf))
                , buf_size_(boost::asio::buffer_size(buf))
            {
                this->setg(buf_, buf_, buf_ + buf_size_);
                this->setp(buf_ + buf_size_, buf_ + buf_size_);
            }

            ArchiveBuffer(
                boost::asio::const_buffer const & buf, 
                std::size_t data_size)
                : buf_((char_type *)boost::asio::buffer_cast<char_type const *>(buf))
                , buf_size_(boost::asio::buffer_size(buf))
            {
                this->setg(buf_, buf_, buf_ + data_size);
                this->setp(buf_ + data_size, buf_ + buf_size_);
            }

            ArchiveBuffer(
                boost::asio::mutable_buffer const & buf)
                : buf_(boost::asio::buffer_cast<char_type *>(buf))
                , buf_size_(boost::asio::buffer_size(buf))
            {
                this->setg(buf_, buf_, buf_);
                this->setp(buf_, buf_ + buf_size_);
            }

            ArchiveBuffer(
                boost::asio::mutable_buffer const & buf, 
                std::size_t data_size)
                : buf_(boost::asio::buffer_cast<char_type *>(buf))
                , buf_size_(boost::asio::buffer_size(buf))
            {
                this->setg(buf_, buf_, buf_ + data_size);
                this->setp(buf_ + data_size, buf_ + buf_size_);
            }

            /**
            ���ض����������

            �����ƶ���ָ�롣
            */
            boost::asio::const_buffers_1 data(
                std::size_t n = std::size_t(-1))
            {
                if (n > (size_t)(this->pptr() - this->gptr()))
                    n = this->pptr() - this->gptr();
                return boost::asio::const_buffers_1(this->gptr(), n);
            }

            std::size_t size()
            {
                return this->pptr() - this->gptr();
            }

            /**
            ��д����������ύ�Ķ�����

            ׷�Ӵ�дָ�뿪ʼ�� @c n �ֽ����ݵ������棬дָ����ǰ�ƶ� @c n �ֽڡ�
            ��Ҫ���ⲿд��ʵ�����ݣ�Ȼ���ύ
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
            �Ӷ������Ƴ�����

            ��ָ����ǰ�ƶ� @c n �ֽڡ�
            ��Ҫ���ⲿ����ʵ�����ݣ�Ȼ���Ƴ���Ӧ����
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

            /**
            ��д������׼��һ�οռ�

            �����ƶ�дָ�롣
            */
            boost::asio::mutable_buffers_1 prepare(
                std::size_t n)
            {
                if (n > (size_t)(this->epptr() - this->pptr()))
                    n = this->epptr() - this->pptr();
                return boost::asio::mutable_buffers_1(this->pptr(), n);
            }

        protected:
            virtual int_type underflow()
            {
                if (this->gptr() < this->pptr()) {
                    this->setg(buf_, this->gptr(), this->pptr());
                    return traits_type::to_int_type(*this->gptr());
                }
                return traits_type::eof();
            }

            virtual int_type overflow(int_type c)
            {
                throw std::length_error("archive buffer too long");
                return traits_type::not_eof(c);
            }

            virtual pos_type seekoff(
                off_type off, 
                std::ios_base::seekdir dir,
                std::ios_base::openmode mode)
            {
                if (dir == std::ios_base::cur) {
                    pos_type pos = (mode == std::ios_base::in ? this->gptr() : this->pptr()) - buf_;
                    if (off == 0) {
                        return pos;
                    }
                    pos += off;
                    return seekpos(pos, mode);
                } else if (dir == std::ios_base::beg) {
                    return seekpos(off, mode);
                } else if (dir == std::ios_base::end) {
                    assert(off <= 0);
                    return seekpos(pos_type(buf_size_) + off, mode);
                } else {
                    return pos_type(-1);
                }
            }

            virtual pos_type seekpos(
                pos_type position, 
                std::ios_base::openmode mode)
            {
                assert(position != pos_type(-1));
                if (mode == std::ios_base::in) {
                    if (position <= this->pptr() - buf_) {
                        this->setg(this->eback(), this->eback() + position, this->pptr());
                    } else {
                        return pos_type(-1);
                    }
                } else if (mode == std::ios_base::out) {
                    if (position >= this->gptr() - buf_) {
                        this->setp(this->gptr(), this->epptr());
                        this->pbump(buf_ + position - this->gptr());
                    } else {
                        return pos_type(-1);
                    }
                } else {
                    if ((size_t)position <= buf_size_) {
                        this->setg(buf_, buf_ + position, buf_ + position);
                        this->setp(buf_ + position, buf_ + buf_size_);
                    } else {
                        return pos_type(-1);
                    }
                }
                return position;
            }

        private:
            char_type * buf_; ///< �����׵�ַ
            std::size_t buf_size_; ///< �����С

        }; // class ArchiveBuffer

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_ARCHIVE_BUFFER_H_
