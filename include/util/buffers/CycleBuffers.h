// CycleBuffers.h

#ifndef _UTIL_BUFFERS_CYCLE_BUFFERS_H_
#define _UTIL_BUFFERS_CYCLE_BUFFERS_H_

#include "util/buffers/SnapshotBuffer.h"
#include "util/buffers/StlBuffer.h"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template <
            typename Buffer, 
            typename Witch, 
            typename Buffer_Iterator
        >
        struct cycle_buffers_status
        {
            cycle_buffers_status(
                Buffer & buf)
                : at_end(true)
                , pos(0)
                , first(buf)
            {
            }

            cycle_buffers_status(
                Buffer & buf, 
                cycle_buffers_status const & other)
                : at_end(other.at_end)
                , pos(other.pos)
                , first(buf, other.first)
                , begin_remainder(other.begin_remainder)
            {
            }

            cycle_buffers_status(
                Buffer & buf, 
                cycle_buffers_status<Buffer, typename SwitchWitch<Witch>::type, Buffer_Iterator> & other)
                : at_end(other.at_end)
                , pos(other.pos)
                , first(buf, other.first)
                , begin_remainder(other.begin_remainder)
            {
            }

            cycle_buffers_status & operator=(
                cycle_buffers_status const & other)
            {
                at_end = other.at_end;
                pos = other.pos;
                first = other.first;
                begin_remainder = other.begin_remainder;
                return *this;
            }

            cycle_buffers_status & operator=(
                cycle_buffers_status<Buffer, typename SwitchWitch<Witch>::type, Buffer_Iterator> & other)
            {
                at_end = other.at_end;
                pos = other.pos;
                first = other.first;
                begin_remainder = other.begin_remainder;
                return *this;
            }

            typename Buffer::pos_type position() const
            {
                return pos + (typename Buffer::off_type)first.offset();
            }

            void position(typename Buffer::pos_type p)
            {
                pos = p - (typename Buffer::off_type)first.offset();
            }

            bool at_end;
            typename Buffer::pos_type pos;
            StlBuffer<Witch, typename Buffer::char_type, typename Buffer::traits_type> first;
            Buffer_Iterator begin_remainder;
        };

        template <
            typename Witch, 
            typename Buffer_Iterator
        >
        struct cycle_buffers_position
        {
            typedef typename WitchBuffer<Witch>::type buffer_type;

            cycle_buffers_position()
                : at_end(true)
            {
            }

            cycle_buffers_position(
                cycle_buffers_position const & other)
                : at_end(other.at_end)
                , first(other.first)
                , begin_remainder(other.begin_remainder)
            {
            }

            template <
                typename Buffer
            >
            cycle_buffers_position(
                cycle_buffers_status<Buffer, Witch, Buffer_Iterator> const & other)
                : at_end(other.at_end)
                , first(other.first)
                , begin_remainder(other.begin_remainder)
            {
            }

            friend bool operator==(
                cycle_buffers_position const & l, 
                cycle_buffers_position const & r)
            {
                using namespace boost::asio;

                if (l.at_end && r.at_end)
                    return true;
                return !l.at_end && !r.at_end
                    && buffer_cast<void const *>(l.first) == buffer_cast<void const *>(r.first)
                    && buffer_size(l.first) == buffer_size(r.first)
                    && l.begin_remainder == r.begin_remainder;
            }

            bool at_end;
            buffer_type first;
            Buffer_Iterator begin_remainder;
        };

        template <
            typename Witch, 
            typename Buffer_Iterator
        >
        class cycle_buffers_iterator
            : public boost::iterator_facade<
                cycle_buffers_iterator<Witch, Buffer_Iterator>,
                typename WitchBuffer<Witch>::type const, 
                boost::forward_traversal_tag, 
                typename WitchBuffer<Witch>::type const
            >
        {
        public:
            typedef typename SwitchWitch<Witch>::type Witch2;

            typedef typename WitchVoidPointer<Witch>::type void_pointer;
            typedef typename WitchVoidPointer<Witch2>::type void_pointer2;

            // Default constructor creates an end iterator.
            cycle_buffers_iterator()
            {
            }

            // Construct with a buffer for the first entry and an iterator
            // range for the remaining entries.
            cycle_buffers_iterator(
                Buffer_Iterator begin, 
                Buffer_Iterator end, 
                cycle_buffers_position<Witch, Buffer_Iterator> const & left,
                cycle_buffers_position<Witch2, Buffer_Iterator> const & right, 
                size_t max_size_ = 65536)
                : begin_(begin)
                , end_(end)
                , left_(left)
                , right_(right)
                , offset_(0)
                , max_size_(max_size_)
            {
                using namespace boost::asio;
                left_.first = buffer(left_.first, max_size_);
                if (buffer_size(left_.first) == 0) {
                    increment();
                }
            }

        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                using namespace boost::asio;

                if (left_.at_end) {
                    return;
                }
                offset_ += buffer_size(left_.first);
                if (offset_ >= max_size_) {
                    left_.at_end = true;
                    return;
                }
                void_pointer lptr = (void_pointer)(
                    (char const *)buffer_cast<void_pointer>(left_.first) + buffer_size(left_.first));
                left_.first = buffer(lptr, 0);
                while (!left_.at_end && buffer_size(left_.first) == 0) {
                    if (left_.begin_remainder == right_.begin_remainder) {
                        void_pointer lptr = buffer_cast<void_pointer>(left_.first);
                        void_pointer2 rptr = buffer_cast<void_pointer2>(right_.first);
                        if (lptr < rptr) {
                            size_t left = (char const *)rptr - (char const *)lptr;
                            if (offset_ + left > max_size_)
                                left = max_size_ - offset_;
                            left_.first = buffer(lptr , left);
                            break;
                        } else if (rptr < lptr) {
                            // 下一段
                        } else {
                            left_.at_end = true;
                            break;
                        }
                    }
                    left_.first = buffer(*left_.begin_remainder++, max_size_ - offset_);
                    if (left_.begin_remainder == end_)
                        left_.begin_remainder = begin_;
                    if (left_.begin_remainder == right_.begin_remainder) {
                        size_t real_size = buffer_cast<const char*>(right_.first) - buffer_cast<const char*>(left_.first);
                        left_.first = buffer(left_.first, real_size);
                    }
                } // while
            }

            bool equal(
                const cycle_buffers_iterator& other) const
            {
                return left_ == other.left_;
            }

            typename WitchBuffer<Witch>::type dereference() const
            {
                return left_.first;
            }

            Buffer_Iterator begin_;
            Buffer_Iterator end_;
            cycle_buffers_position<Witch, Buffer_Iterator> left_;
            cycle_buffers_position<Witch2, Buffer_Iterator> right_;
            std::size_t offset_;
            std::size_t max_size_;
        };

        template <
            typename Buffers, 
            typename Elem = char, 
            typename Traits = std::char_traits<Elem>
        >
        class CycleBuffers
            : public StlStream<Elem, Traits>
        {
        public:
            typedef typename StlStream<Elem, Traits>::char_type char_type;

            typedef typename StlStream<Elem, Traits>::int_type int_type;

            typedef typename StlStream<Elem, Traits>::traits_type traits_type;

            typedef typename StlStream<Elem, Traits>::pos_type pos_type;
 
            typedef typename StlStream<Elem, Traits>::off_type off_type;
 
            typedef Buffers buffers_type;

            typedef CycleBuffers cycle_buffers_type;

            typedef StlStream<Elem, Traits> stream_type;

            typedef boost::asio::mutable_buffer value_type;

            typedef typename buffers_type::const_iterator buffer_const_iterator;

            typedef cycle_buffers_status<
                cycle_buffers_type, detail::_read, buffer_const_iterator> read_buffer_status;

            typedef cycle_buffers_status<
                cycle_buffers_type, detail::_write, buffer_const_iterator> write_buffer_status;

            // A forward-only iterator type that may be used to read elements.
            typedef cycle_buffers_iterator<detail::_read, buffer_const_iterator>
                const_buffer_iterator;

            typedef cycle_buffers_iterator<detail::_write, buffer_const_iterator>
                mutable_buffer_iterator;

            class ConstBuffers
            {
            public:
                // The type for each element in the list of buffers.
                typedef boost::asio::const_buffer value_type;

                // A forward-only iterator type that may be used to read elements.
                typedef const_buffer_iterator const_iterator;

                explicit ConstBuffers(
                    CycleBuffers const & buffers, 
                    size_t max_size_ = size_t(-1))
                    : buffers_(buffers)
                    , max_size_(max_size_)
                {
                }

                // Get a forward-only iterator to the first element.
                const_iterator begin() const
                {
                    return buffers_.rbegin(max_size_);
                }

                // Get a forward-only iterator for one past the last element.
                const_iterator end() const
                {
                    return buffers_.rend();
                }

            private:
                CycleBuffers const buffers_;
                size_t max_size_;
            };

            class MutableBuffers
            {
            public:
                // The type for each element in the list of buffers.
                typedef boost::asio::mutable_buffer value_type;

                // A forward-only iterator type that may be used to read elements.
                typedef mutable_buffer_iterator const_iterator;

                explicit MutableBuffers(
                    CycleBuffers const & buffers, 
                    size_t max_size_)
                    : buffers_(buffers)
                    , max_size_(max_size_)
                {
                }

                // Get a forward-only iterator to the first element.
                const_iterator begin() const
                {
                    return buffers_.wbegin(max_size_);
                }

                // Get a forward-only iterator for one past the last element.
                const_iterator end() const
                {
                    return buffers_.wend();
                }

            private:
                CycleBuffers const buffers_;
                size_t max_size_;
            };

            typedef ConstBuffers const_buffers_type;
            typedef MutableBuffers mutable_buffers_type;

        public:
            CycleBuffers()
                : write_(*this)
                , read_(*this)
            {
                capacity_ = 0;
                reset();
            }

            CycleBuffers(
                Buffers const & buffers)
                : buffers_(buffers)
                , write_(*this)
                , read_(*this)
            {
                capacity_ = 0;
                typedef typename Buffers::const_iterator const_iterator;
                for (const_iterator iter = buffers_.begin(); iter != buffers_.end(); ++iter) {
                    boost::asio::const_buffer buffer(*iter);
                    capacity_ += boost::asio::buffer_size(buffer);
                }
                reset();
            }

            // Copy constructor.
            CycleBuffers(
                const CycleBuffers & other)
                : buffers_(other.buffers_)
                , write_(*this, other.write_)
                , read_(*this, other.read_)
                , capacity_(other.capacity_)
            {
                write_.begin_remainder = buffers_.begin();
                read_.begin_remainder = buffers_.begin();
                typename Buffers::const_iterator first = other.buffers_.begin();
                typename Buffers::const_iterator second = other.write_.begin_remainder;
                std::advance(write_.begin_remainder, std::distance(first, second));
                typename Buffers::const_iterator third = other.read_.begin_remainder;
                std::advance(read_.begin_remainder, std::distance(first, third));
            }

            // Assignment operator.
            CycleBuffers & operator=(
                const CycleBuffers& other)
            {
                buffers_ = other.buffers_;
                write_ = other.write_;
                read_ = other.read_;
                write_.begin_remainder = buffers_.begin();
                read_.begin_remainder = buffers_.begin();
                typename Buffers::const_iterator first = other.buffers_.begin();
                typename Buffers::const_iterator second = other.write_.begin_remainder;
                std::advance(write_.begin_remainder, std::distance(first, second));
                typename Buffers::const_iterator third = other.read_.begin_remainder;
                std::advance(read_.begin_remainder, std::distance(first, third));
                capacity_ = other.capacity_;
                return *this;
            }

        public:
            mutable_buffers_type prepare() const
            {
                return mutable_buffers_type(*this, out_avail());
            }

            mutable_buffers_type prepare(
                size_t size) const
            {
                if (size > this->out_avail()) 
                    throw std::length_error("util::buffer::CycleStreamBuffers too long");
                return mutable_buffers_type(*this, size);
            }

            const_buffers_type data() const
            {
                return const_buffers_type(*this, in_avail());
            }

            const_buffers_type data(
                size_t size) const
            {
                if (size > this->in_avail()) 
                    throw std::length_error("util::buffer::CycleStreamBuffers too long");
                return const_buffers_type(*this, size);
            }

        public:
            SnapshotBuffer snapshot() const
            {
                return SnapshotBuffer(data(), in_avail());
            }

            SnapshotBuffer snapshot(
                size_t size) const
            {
                if (size > in_avail())
                    size = in_avail();
                return SnapshotBuffer(data(), size);
            }

            SnapshotBuffer snapshot(
                size_t offset, 
                size_t size) const
            {
                if (offset >= in_avail()) {
                    return SnapshotBuffer();
                } else if (offset + size >= in_avail()) {
                    size = in_avail() - offset;
                }
                return SnapshotBuffer(data(), offset, size);
            }

        public:
            size_t capacity() const
            {
                return capacity_;
            }

            size_t in_avail() const
            {
                return write_.position() - read_.position();
            }

            size_t out_avail() const
            {
                return capacity_ - in_avail();
            }

            size_t in_position() const
            {
                return read_.position();
            }

            size_t out_position() const
            {
                return write_.position();
            }

        public:
            // Get a forward-only iterator to the first element.
            mutable_buffer_iterator wbegin() const
            {
                return mutable_buffer_iterator(buffers_.begin(), buffers_.end(), write_, read_);
            }

            mutable_buffer_iterator wbegin(
                size_t max_size_) const
            {
                return mutable_buffer_iterator(buffers_.begin(), buffers_.end(), write_, read_, max_size_);
            }

            // Get a forward-only iterator for one past the last element.
            mutable_buffer_iterator wend() const
            {
                return mutable_buffer_iterator();
            }

            // Get a forward-only iterator to the first element.
            const_buffer_iterator rbegin() const
            {
                return const_buffer_iterator(buffers_.begin(), buffers_.end(), read_, write_);
            }

            const_buffer_iterator rbegin(
                size_t max_size_) const
            {
                return const_buffer_iterator(buffers_.begin(), buffers_.end(), read_, write_, max_size_);
            }

            // Get a forward-only iterator for one past the last element.
            const_buffer_iterator rend() const
            {
                return const_buffer_iterator();
            }

        public:
            // Consume the specified number of bytes from the buffers.
            void commit(
                std::size_t size)
            {
                size_t rpos = in_position();
                size_t wpos = out_position();
                shift(write_, read_, size);
                check();
                assert(rpos == in_position());
                assert(wpos + size == out_position());
            }

            void consume(
                std::size_t size)
            {
                size_t rpos = in_position();
                size_t wpos = out_position();
                shift(read_, write_, size);
                check();
                assert(rpos + size == in_position());
                assert(wpos == out_position());
            }

            void reset()
            {
                write_.at_end = false;
                write_.begin_remainder = buffers_.begin();
                read_.at_end = true;
                read_.begin_remainder = buffers_.begin();
                if (buffers_.begin() == buffers_.end()) {
                    write_.at_end = true;
                } else {
                    write_.first = boost::asio::buffer(*buffers_.begin());
                    ++write_.begin_remainder;
                    if (write_.begin_remainder == buffers_.end())
                        write_.begin_remainder = buffers_.begin();
                    read_.first = boost::asio::buffer(*buffers_.begin(), 0);
                    ++read_.begin_remainder;
                    if (read_.begin_remainder == buffers_.end())
                        read_.begin_remainder = buffers_.begin();
                }
            }

        private:
            template <
                typename Witch
            >
            void shift(
                cycle_buffers_status<cycle_buffers_type, Witch, buffer_const_iterator> & left, 
                cycle_buffers_status<cycle_buffers_type, typename SwitchWitch<Witch>::type, buffer_const_iterator> & right, 
                std::size_t size)
            {
                using namespace boost::asio;
                // Remove buffers from the start until the specified size is reached.
                while (size > 0 && !left.at_end) {
                    normalize(left, right);
                    std::size_t this_size = buffer_size(left.first);
                    if (this_size > size) {
                        this_size = size;
                    }
                    size -= this_size;
                    left.first.consume(this_size);
                    right.at_end = false;
                }
                normalize(left, right);
            }

            template <
                typename Witch
            >
            void normalize(
                cycle_buffers_status<cycle_buffers_type, Witch, buffer_const_iterator> & left, 
                cycle_buffers_status<cycle_buffers_type, typename SwitchWitch<Witch>::type, buffer_const_iterator> & right)
            {
                // Remove any more empty buffers at the start.
                while (!left.at_end && left.first.size() == 0) {
                    if (left.begin_remainder == right.begin_remainder) {
                        if (left.first.ptr() < right.first.ptr()) {
                            size_t real_size = right.first.ptr() - left.first.ptr();
                            left.first.commit(real_size);
                            right.pos += right.first.drop_back();
                            return;
                        } else if (right.first.ptr() < left.first.ptr()) {
                            std::size_t off = left.first.offset();
                            right.first.commit(off);
                            right.at_end = false;
                        } else {
                            // 左指针在右指针后面，追上了，所以结束
                            if (left.first.before(right.first)) {
                                left.at_end = true;
                                return;
                            }
                        }
                    }
                    // 左指针在右指针前面，左指针离开，右指针跟进
                    std::size_t off = left.first.offset();
                    left.pos += off;
                    left.first = boost::asio::buffer(*left.begin_remainder++);
                    if (left.begin_remainder == buffers_.end())
                        left.begin_remainder = buffers_.begin();
                    if (left.begin_remainder == right.begin_remainder) {
                        // 左指针将要追上右指针，这一块数据只有一部分可访问
                        size_t real_size = right.first.ptr() - left.first.ptr();
                        left.first.limit_size(real_size);
                        right.pos += right.first.drop_back();
                    }
                }
            }

        private:
            virtual int_type underflow()
            {
                if (this->gptr() != this->pptr()) {
                    normalize(read_, write_);
                    if (this->gptr() != this->pptr()) {
                        check();
                        return traits_type::to_int_type(*this->gptr());
                    } else {
                        //assert(0);
                        return traits_type::eof();
                    }
                } else {
                    // assert(0);
                    return traits_type::eof();
                }
            }

            virtual int_type overflow(
                int_type c)
            {
                if (!traits_type::eq_int_type(c, traits_type::eof()))
                {
                    //assert(this->gptr() == this->pptr());
                    normalize(write_, read_);
                    assert(this->gptr() != this->pptr());
                    *this->pptr() = traits_type::to_char_type(c);
                    this->pbump(1);
                    check();
                    return c;
                }
                return traits_type::not_eof(c);
            }

            virtual pos_type seekoff(
                off_type off, 
                std::ios_base::seekdir dir,
                std::ios_base::openmode mode)
            {
                pos_type pos = 0;
                if (dir == std::ios_base::beg) {
                    pos = off;
                } else if (dir == std::ios_base::cur) {
                    if (mode == std::ios_base::in) {
                        pos = read_.position() + off;
                    } else if (mode == std::ios_base::out) {
                        pos = write_.position() + off;
                    } else {
                        return pos_type(-1);
                    }
                } else {
                    return pos_type(-1);
                }
                check();
                return seekpos(pos, mode);
            }

            virtual pos_type seekpos(
                pos_type pos, 
                std::ios_base::openmode mode)
            {
                if (mode == std::ios_base::in) {
                    if (pos > write_.position() || pos + off_type(capacity_) < write_.position()) {
                        return pos_type(-1);
                    }
                    if (pos > read_.position()) {
                        consume(pos - read_.position());
                    } else if (pos < read_.position()) {
                        write_.pos -= write_.first.split(boost::asio::buffer(*write_.begin_remainder), read_.first);
                        read_.begin_remainder = write_.begin_remainder;
                        read_.position(write_.position());
                        read_.at_end = false;
                        write_.pos += off_type(capacity_);
                        shift(read_, write_, pos + off_type(capacity_) - read_.position());
                        read_.pos -= off_type(capacity_);
                        write_.pos -= off_type(capacity_);
                    }
                    assert(pos == read_.position());
                } else if (mode == std::ios_base::out) {
                    if (pos < read_.position() || pos > off_type(capacity_) + read_.position()) {
                        return pos_type(-1);
                    }
                    if (pos > write_.position()) {
                        consume(pos - write_.position());
                    } else if (pos < write_.position()) {
                        read_.pos -= read_.first.split(boost::asio::buffer(*read_.begin_remainder), write_.first);
                        write_.begin_remainder = read_.begin_remainder;
                        write_.position(read_.position());
                        write_.at_end = false;
                        check();
                        shift(write_, read_, pos - write_.position());
                    }
                    assert(pos == write_.position());
                } else { // mode == std::ios_base::in | std::ios_base::out
                    reset();
                    pos_type pos1 = pos % capacity_;
                    shift(write_, read_, pos1);
                    shift(read_, write_, pos1);
                    write_.position(pos);
                    read_.position(pos);
                }
                return pos;
            }

            virtual int sync()
            {
                if (read_.begin_remainder == write_.begin_remainder) {
                    if (read_.at_end && read_.first.ptr() < write_.first.ptr()) {
                        read_.at_end = false;
                    } else if (write_.at_end && write_.first.ptr() < read_.first.ptr()) {
                        write_.at_end = false;
                    }
                }
                check();
                return 0;
            }

            void check()
            {
                assert((size_t)(write_.position() - read_.position()) <= capacity_);
                if (read_.begin_remainder == write_.begin_remainder) {
                    assert(this->egptr() == this->pbase() || this->epptr() == this->eback());
                }
            }

        private:
            buffers_type buffers_;
            write_buffer_status write_;
            read_buffer_status read_;
            size_t capacity_;
        };

    } // namespace buffer
} // namespace util

#endif // _UTIL_BUFFERS_CYCLE_BUFFERS_H_
