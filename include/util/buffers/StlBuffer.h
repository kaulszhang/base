// StlBuffer.h

#ifndef _UTIL_BUFFERS_STL_BUFFER_H_
#define _UTIL_BUFFERS_STL_BUFFER_H_

#include <boost/asio/buffer.hpp>

#include <streambuf>

namespace util
{
    namespace buffers
    {

        template <
            class _Witch
        >
        struct WitchVoidPointer;

        template <
            class _Witch
        >
        struct WitchBuffer;

        template <
            class _Witch
        >
        struct WitchBuffers;

        template <
            class _Witch
        >
        struct SwitchWitch;

        namespace detail
        {
            struct _write{};
            struct _read{};
        }

        template <
        >
        struct WitchVoidPointer<detail::_read>
        {
            typedef void const * type;
        };

        template <
        >
        struct WitchVoidPointer<detail::_write>
        {
            typedef void * type;
        };

        template <
        >
        struct WitchBuffer<detail::_read>
        {
            typedef boost::asio::const_buffer type;
        };

        template <
        >
        struct WitchBuffer<detail::_write>
        {
            typedef boost::asio::mutable_buffer type;
        };

        template <
        >
        struct WitchBuffers<detail::_read>
        {
            typedef boost::asio::const_buffers_1 type;
        };

        template <
        >
        struct WitchBuffers<detail::_write>
        {
            typedef boost::asio::mutable_buffers_1 type;
        };

        template <
        >
        struct SwitchWitch<detail::_write>
        {
            typedef detail::_read type;
        };

        template <
        >
        struct SwitchWitch<detail::_read>
        {
            typedef detail::_write type;
        };

        template <
            class _Witch, 
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class StlBuffer;

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class StlStream
            : public std::basic_streambuf<_Elem, _Traits>
        {
        public:
            typedef typename std::basic_streambuf<_Elem, _Traits>::char_type char_type;

        private:
            //template <
            //    class _Witch
            //>
            //friend class StlBuffer<StlStream, _Witch>;
            friend class StlBuffer<detail::_read, _Elem, _Traits>;
            friend class StlBuffer<detail::_write, _Elem, _Traits>;

            void set_buffer(
                char_type * beg, 
                char_type * cur, 
                char_type * end, 
                detail::_write const &)
            {
                this->setp(beg, end);
                this->pbump(static_cast<int>(cur - beg));
            }

            void set_buffer(
                char_type * beg, 
                char_type * cur, 
                char_type * end, 
                detail::_read const &)
            {
                this->setg(beg, cur, end);
            }

            void get_buffer(
                char_type *& beg, 
                char_type *& cur, 
                char_type *& end, 
                detail::_write const &)
            {
                beg = this->pbase();
                cur = this->pptr();
                end = this->epptr();
            }

            void get_buffer(
                char_type *& beg, 
                char_type *& cur, 
                char_type *& end, 
                detail::_read const &)
            {
                beg = this->eback();
                cur = this->gptr();
                end = this->egptr();
            }

        };

        template <
            class _Witch, 
            typename _Elem, 
            typename _Traits
        >
        class StlBuffer
            : public _Witch
        {
        public:
            typedef _Elem char_type;
            typedef _Traits traits_type;
            typedef typename WitchVoidPointer<_Witch>::type pointer_type;

            StlBuffer(
                StlStream<_Elem, _Traits> & stl_buf)
                : stl_buf_(stl_buf)
                , end_(NULL)
            {
                char_type * p = NULL;
                stl_buf_.set_buffer(p, p, p, *this);
            }

            StlBuffer(
                StlStream<_Elem, _Traits> & stl_buf, 
                StlBuffer const & other)
                : stl_buf_(stl_buf)
                , end_(other.end_)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                other.stl_buf_.get_buffer(beg, cur, end, other);
                stl_buf_.set_buffer(beg, cur, end, *this);
            }

            friend class StlBuffer<typename SwitchWitch<_Witch>::type, _Elem, _Traits>;

            StlBuffer(
                StlStream<_Elem, _Traits> & stl_buf, 
                StlBuffer<typename SwitchWitch<_Witch>::type, _Elem, _Traits> & other)
                : stl_buf_(stl_buf)
                , end_(other.end_)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                other.stl_buf_.get_buffer(beg, cur, end, other);
                end_ = other.end_;
                stl_buf_.set_buffer(beg, cur, end, *this);
                end = cur;
                other.stl_buf_.set_buffer(beg, cur, end, other);
            }

            StlBuffer & operator=(
                StlBuffer const & other)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                other.stl_buf_.get_buffer(beg, cur, end, other);
                stl_buf_.set_buffer(beg, cur, end, *this);
                end_ = other.end_;
                return *this;
            }

            void operator=(
                boost::asio::mutable_buffer const & buf)
            {
                char_type * beg = boost::asio::buffer_cast<char_type *>(buf);
                char_type * cur = beg;
                char_type * end = beg + boost::asio::buffer_size(buf);
                stl_buf_.set_buffer(beg, cur, end, *this);
                end_ = end;
            }

            void operator=(
                boost::asio::const_buffer const & buf)
            {
                char_type * beg = (char_type *)boost::asio::buffer_cast<char_type const *>(buf);
                char_type * cur = beg;
                char_type * end = beg + boost::asio::buffer_size(buf);
                stl_buf_.set_buffer(beg, cur, end, *this);
                end_ = end;
            }

            void consume(
                std::size_t n)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                stl_buf_.get_buffer(beg, cur, end, *this);
                cur += n;
                assert(cur <= end);
                stl_buf_.set_buffer(beg, cur, end, *this);
            }

            std::size_t drop_back()
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                stl_buf_.get_buffer(beg, cur, end, *this);
                std::size_t n = cur - beg;
                beg = cur;
                stl_buf_.set_buffer(beg, cur, end, *this);
                return n;
            }

            void commit(
                std::size_t n)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                stl_buf_.get_buffer(beg, cur, end, *this);
                end += n;
                if (end > end_)
                    end_ = end;
                stl_buf_.set_buffer(beg, cur, end, *this);
            }
           
            void limit_size(
                std::size_t n)
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                stl_buf_.get_buffer(beg, cur, end, *this);
                if (cur + n < end)
                    end = cur + n;
                stl_buf_.set_buffer(beg, cur, end, *this);
            }

            char_type * ptr() const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                stl_buf_.get_buffer(beg, cur, end, *this);
                (void)beg;
                (void)end;
                return cur;
            }

            std::size_t offset() const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                stl_buf_.get_buffer(beg, cur, end, *this);
                (void)end;
                return cur - beg;
            }

            std::size_t size() const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                stl_buf_.get_buffer(beg, cur, end, *this);
                (void)beg;
                return end - cur;
            }

            std::size_t total() const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                stl_buf_.get_buffer(beg, cur, end, *this);
                (void)cur;
                return end - beg;
            }

            std::size_t split(
                //boost::asio::mutable_buffer const & buf, 
                boost::asio::const_buffer const & buf, 
                StlBuffer<typename SwitchWitch<_Witch>::type, _Elem, _Traits> & other)
            {
                char_type * buf_beg = (char_type *)boost::asio::buffer_cast<char_type const *>(buf);
                char_type * buf_end = buf_beg + boost::asio::buffer_size(buf);
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                stl_buf_.get_buffer(beg, cur, end, *this);
                assert(buf_beg <= beg && end <= buf_end);
                std::size_t off = beg - buf_beg;
                beg = buf_beg;
                end = cur;
                stl_buf_.set_buffer(beg, cur, end, *this);
                end_ = end;
                beg = cur;
                end = buf_end;
                other.stl_buf_.set_buffer(beg, cur, end, other);
                other.end_ = end;
                return off;
            }

            bool before(
                StlBuffer<typename SwitchWitch<_Witch>::type, _Elem, _Traits> const & other) const
            {
                char_type * beg1 = 0;
                char_type * cur1 = 0;
                char_type * end1 = 0;
                stl_buf_.get_buffer(beg1, cur1, end1, *this);
                char_type * beg2 = 0;
                char_type * cur2 = 0;
                char_type * end2 = 0;
                other.stl_buf_.get_buffer(beg2, cur2, end2, other);
                (void)cur1;
                (void)cur2;
                assert(beg1 == end2 || beg2 == end1);
                return beg1 < beg2 || end1 < end2;
            }

            bool operator==(
                StlBuffer const & r) const
            {
                StlBuffer const & l = *this;
                char_type * l_beg = 0;
                char_type * l_cur = 0;
                char_type * l_end = 0;
                l.stl_buf_.get_buffer(l_beg, l_cur, l_end, l);
                char_type * r_beg = 0;
                char_type * r_cur = 0;
                char_type * r_end = 0;
                r.stl_buf_.get_buffer(r_beg, r_cur, r_end, r);
                return l_cur == r_cur && l_end == r_end;
            }
            
            operator typename WitchBuffer<_Witch>::type() const
            {
                char_type * beg = 0;
                char_type * cur = 0;
                char_type * end = 0;
                stl_buf_.get_buffer(beg, cur, end, *this);
                (void)beg;
                return typename WitchBuffer<_Witch>::type(cur, end - cur);
            }

        private:
            StlStream<_Elem, _Traits> & stl_buf_;
            char_type * end_;
        };

    }
}

namespace boost
{
    namespace asio
    {
        namespace detail
        {
            template <
                class _Witch, 
                typename _Elem, 
                typename _Traits
            >
            inline typename util::buffers::WitchVoidPointer<_Witch>::type buffer_cast_helper(
                util::buffers::StlBuffer<_Witch, _Elem, _Traits> const & b)
            {
                return b.ptr();
            }

            template <
                class _Witch, 
                typename _Elem, 
                typename _Traits
            >
            inline std::size_t buffer_size_helper(
                util::buffers::StlBuffer<_Witch, _Elem, _Traits> const & b)
            {
                return b.size();
            }
        }

        template <
            typename PointerToPodType, 
            class _Witch, 
            typename _Elem, 
            typename _Traits
        >
        inline PointerToPodType buffer_cast(
            util::buffers::StlBuffer<_Witch, _Elem, _Traits> const & b)
        {
            return static_cast<PointerToPodType>(detail::buffer_cast_helper(b));
        }

        template <
            class _Witch, 
            typename _Elem, 
            typename _Traits
        >
        inline std::size_t buffer_size(
            util::buffers::StlBuffer<_Witch, _Elem, _Traits>const & b)
        {
            return detail::buffer_size_helper(b);
        }

        template <
            class _Witch, 
            typename _Elem, 
            typename _Traits
        >
        inline typename util::buffers::WitchBuffers<_Witch>::type buffer(
            const util::buffers::StlBuffer<_Witch, _Elem, _Traits> & b)
        {
            return typename util::buffers::WitchBuffers<_Witch>::type(
                detail::buffer_cast_helper(b), buffer_size_helper(b));
        }

        template <
            class _Witch, 
            typename _Elem, 
            typename _Traits
        >
        inline typename util::buffers::WitchBuffers<_Witch>::type buffer(
            const util::buffers::StlBuffer<_Witch, _Elem, _Traits> & b, 
            std::size_t max_size_in_bytes)
        {
            return typename util::buffers::WitchBuffers<_Witch>::type(
                typename util::buffers::WitchBuffer<_Witch>::type(detail::buffer_cast_helper(b), buffer_size_helper(b)), max_size_in_bytes);
        }
    }
}

#endif // _UTIL_BUFFERS_STL_BUFFER_H_
