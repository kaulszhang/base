// BuffersPosition.h

#ifndef _UTIL_BUFFERS_BUFFERS_POSITION_H_
#define _UTIL_BUFFERS_BUFFERS_POSITION_H_

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template<
            typename Buffer
        >
        struct BufferByteType
        {
            typedef char type;
        };

        template<
        >
        struct BufferByteType<boost::asio::const_buffer>
        {
            typedef char const type;
        };

        template <
            typename BufferIterator
        >
        struct BuffersLimit
        {
            BuffersLimit(
                BufferIterator const & beg, 
                BufferIterator const & end)
                : beg(beg)
                , end(end)
            {
            }

            BuffersLimit()
            {
            }

            BufferIterator beg;
            BufferIterator end;
        };

        template <
            typename Buffer, 
            typename BufferIterator
        >
        class BuffersPosition
        {
        public:
            typedef typename BufferByteType<Buffer>::type Byte;

            typedef BuffersLimit<BufferIterator> Limit;

        public:
            BuffersPosition(
                Limit const & limit, 
                BufferIterator const & iter, 
                size_t off, 
                bool has_end = false)
                : iter_(iter)
                , skipped_bytes_(0)
                , at_end_(false)
            {
                if (iter_ == limit.end) {
                    at_end_ = true;
                } else {
                    buf_ = *iter_;
                    buf_ = buf_ + off;
                    if (!has_end) {
                        set_end(limit);
                    }
                }
            }

            BuffersPosition(
                Limit const & limit, 
                BufferIterator const & iter, 
                bool has_end = false)
                : iter_(iter)
                , skipped_bytes_(0)
                , at_end_(false)
            {
                if (iter_ == limit.end) {
                    at_end_ = true;
                } else {
                    buf_ = *iter_;
                    if (!has_end) {
                        set_end(limit);
                    }
                }
            }

            BuffersPosition(
                Limit const & limit, 
                bool has_end = false)
                : iter_(limit.beg)
                , skipped_bytes_(0)
                , at_end_(false)
            {
                if (iter_ == limit.end) {
                    at_end_ = true;
                } else {
                    buf_ = *iter_;
                }
                if (!has_end) {
                    set_end(limit);
                }
            }

            BuffersPosition()
                : skipped_bytes_(0)
                , at_end_(true)
            {
            }

        public:
            void set_end(
                Limit const & limit)
            {
                if (boost::asio::buffer_size(buf_) == 0)
                    normalize(limit);
            }

            void set_end(
                Limit const & limit, 
                BuffersPosition const & end)
            {
                if (boost::asio::buffer_size(buf_) == 0)
                    normalize(limit, end);
                if (!at_end_ && iter_ == end.iter_) {
                    std::ptrdiff_t size = boost::asio::buffer_cast<char const *>(end.buf_)
                        - boost::asio::buffer_cast<char const *>(buf_);
                    if (size <= 0) {
                        size = 0;
                        at_end_ = true;
                    }
                    // when at end, we should also clear buf_, because we will use (buf_.size() == 0) as end mark in increment_bytes
                    buf_ = boost::asio::buffer(buf_, size);
                }
            }

        public:
            bool at_end() const
            {
                return at_end_;
            }

            bool equal(
                BuffersPosition const & r) const
            {
                if (at_end_ && r.at_end_)
                    return true;
                return at_end_ == r.at_end_ 
                    && iter_ == r.iter_ 
                    && boost::asio::buffer_cast<void const *>(buf_) == boost::asio::buffer_cast<void const *>(r.buf_);
            }

            Byte & dereference_byte() const
            {
                return *boost::asio::buffer_cast<Byte *>(buf_);
            }

            Buffer const & dereference_buffer() const
            {
                return buf_;
            }

            void increment_byte(
                Limit const & limit, 
                BuffersPosition const & end)
            {
                assert(!at_end_);
                if (at_end_) return;
                buf_ = buf_ + 1;
                ++skipped_bytes_;
                if (boost::asio::buffer_size(buf_) == 0) {
                    normalize(limit, end);
                }
            }

            void increment_byte(
                Limit const & limit)
            {
                assert(!at_end_);
                if (at_end_) return;
                buf_ = buf_ + 1;
                ++skipped_bytes_;
                if (boost::asio::buffer_size(buf_) == 0) {
                    normalize(limit);
                }
            }

            void increment_bytes(
                Limit const & limit, 
                BuffersPosition const & end, 
                size_t size)
            {
                assert(!at_end_);
                size_t this_size = boost::asio::buffer_size(buf_);
                assert(this_size > 0);
                while (size > 0 && this_size > 0) {
                    if (this_size > size) {
                        buf_ = buf_ + size;
                        skipped_bytes_ += size;
                        return;
                    }
                    size -= this_size;
                    skipped_bytes_ += this_size;
                    this_size = normalize(limit, end);
                }
                assert(size == 0);
            }

            void increment_bytes(
                Limit const & limit, 
                size_t size)
            {
                assert(!at_end_);
                size_t this_size = boost::asio::buffer_size(buf_);
                assert(this_size > 0);
                while (size > 0 && this_size > 0) {
                    if (this_size > size) {
                        buf_ = buf_ + size;
                        skipped_bytes_ += size;
                        return;
                    }
                    size -= this_size;
                    skipped_bytes_ += this_size;
                    this_size = normalize(limit);
                }
                assert(size == 0);
            }

            void increment_buffer(
                Limit const & limit, 
                BuffersPosition const & end)
            {
                assert(!at_end_);
                if (at_end_) return;
                skipped_bytes_ += boost::asio::buffer_size(buf_);
                normalize(limit, end);
            }

            void increment_buffer(
                Limit const & limit)
            {
                assert(!at_end_);
                if (at_end_) return;
                skipped_bytes_ += boost::asio::buffer_size(buf_);
                normalize(limit);
            }

            size_t skipped_bytes() const
            {
                return skipped_bytes_;
            }

        private:
            size_t normalize(
                Limit const & limit)
            {
                while (++iter_ != limit.end) {
                    buf_ = *iter_;
                    if (boost::asio::buffer_size(buf_) > 0) {
                        return boost::asio::buffer_size(buf_);
                    }
                }
                // when at end, we should also clear buf_, because we will use (buf_.size() == 0) as end mark in increment_bytes
                buf_ = buf_ + boost::asio::buffer_size(buf_);
                at_end_ = true;
                return 0;
            }

            size_t normalize(
                Limit const & limit, 
                BuffersPosition const & end)
            {
                assert(!at_end_);
                if (iter_ == end.iter_) {
                    buf_ = buf_ + boost::asio::buffer_size(buf_);
                    at_end_ = true;
                    return 0;
                }
                while (++iter_ != limit.end) {
                    buf_ = *iter_;
                    if (iter_ == end.iter_) {
                        std::ptrdiff_t size = boost::asio::buffer_cast<char const *>(end.buf_)
                            - boost::asio::buffer_cast<char const *>(buf_);
                        if (size <= 0) {
                            size = 0;
                            at_end_ = true;
                        }
                        // when at end, we should also clear buf_, because we will use (buf_.size() == 0) as end mark in increment_bytes
                        buf_ = boost::asio::buffer(buf_, size);
                        return size;
                    }
                    if (boost::asio::buffer_size(buf_) > 0) {
                        return boost::asio::buffer_size(buf_);
                    }
                }
                // when at end, we should also clear buf_, because we will use (buf_.size() == 0) as end mark in increment_bytes
                buf_ = buf_ + boost::asio::buffer_size(buf_);
                at_end_ = true;
                return 0;
            }

        private:
            BufferIterator iter_;
            Buffer buf_;
            size_t skipped_bytes_;
            bool at_end_;
        };

        template <
            typename Value, 
            typename Iterator
        >
        class Container
        {
        public:
            typedef Value value_type;
            typedef Iterator const_iterator;

        public:
            Container(
                Iterator const & beg, 
                Iterator const & end = Iterator())
                : beg_(beg)
                , end_(end)
            {
            }

            const_iterator begin() const
            {
                return beg_;
            }

            const_iterator end() const
            {
                return end_;
            }

        private:
            Iterator const beg_;
            Iterator const end_;
        };

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_POSITION_H_
