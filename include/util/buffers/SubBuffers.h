// SubBuffer.h

#ifndef _UTIL_BUFFERS_SUB_BUFFERS_H_
#define _UTIL_BUFFERS_SUB_BUFFERS_H_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        template <
            typename Buffer, 
            typename Buffer_Iterator
        >
        class sub_buffers_iterator
            : public boost::iterator_facade<
                sub_buffers_iterator<Buffer, Buffer_Iterator>,
                Buffer const, 
                boost::forward_traversal_tag
            >
        {
        public:
            // Default constructor creates an end iterator.
            sub_buffers_iterator()
            {
                left_ = 0;
            }

            // Construct with a buffer for the first entry and an iterator
            // range for the remaining entries.
            sub_buffers_iterator(
                Buffer_Iterator beg, 
                Buffer_Iterator end, 
                size_t skip, 
                size_t size)
                : beg_(beg)
                , end_(end)
                , buf_(*beg)
                , skip_(skip)
                , left_(size)
            {
                for (cur_ = beg_; skip_ > 0 && cur_ != end_; ++cur_, buf_ = *cur_) {
                    if (boost::asio::buffer_size(buf_) > skip_) {
                        buf_ = buf_ + skip_;
                        skip_ = 0;
                        break;
                    } else if (boost::asio::buffer_size(buf_) == skip_) {
                        skip_ = 0;
                        if (++cur_ != end_)
                            buf_ = *cur_;
                        else
                            left_ = 0;
                        break;
                    }
                    skip_ -= boost::asio::buffer_size(buf_);
                }
                if (skip_)
                    left_ = 0;
				if (left_ < buffer_size(buf_)) {
					buf_ = boost::asio::buffer(buf_, left_);
				}
            }

            sub_buffers_iterator(
                sub_buffers_iterator const & other)
                : beg_(other.beg_)
                , end_(other.end_)
                , cur_(other.cur_)
                , buf_(other.buf_)
                , skip_(other.skip_)
                , left_(other.left_)
            {
            }

        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                if (++cur_ != end_) {
                    left_ -= buffer_size(buf_);
                    buf_ = *cur_;
                    if (left_ < buffer_size(buf_)) {
                        buf_ = boost::asio::buffer(buf_, left_);
                    }
                } else {
                    left_ = 0;
                }
            }

            bool equal(
                const sub_buffers_iterator & other) const
            {
                return left_ == other.left_;
            }

            Buffer const & dereference() const
            {
                return buf_;
            }

            Buffer_Iterator beg_;
            Buffer_Iterator end_;
            Buffer_Iterator cur_;
            Buffer buf_;
            size_t skip_;
            size_t left_;
        };

        template <
            typename BufferSequence
        >
        class SubBuffers
        {
        public:
            typedef typename BufferSequence::value_type value_type;

            typedef sub_buffers_iterator<
                value_type, 
                typename BufferSequence::const_iterator
            > const_iterator;

        public:
            SubBuffers(
                BufferSequence const & buffers, 
                size_t beg, 
                size_t len)
                : buffers_(buffers)
                , beg_(beg)
                , len_(len)
            {
            }

            const_iterator begin() const
            {
                return const_iterator(buffers_.begin(), buffers_.end(), beg_, len_);
            }

            const_iterator end() const
            {
                return const_iterator();
            }

        private:
            BufferSequence buffers_;
            size_t beg_;
            size_t len_;
        };

        template <
            typename BufferSequence
        >
        SubBuffers<BufferSequence> const sub_buffers(BufferSequence const & buffers, 
            size_t beg, 
            size_t len = (size_t)-1)
        {
            return SubBuffers<BufferSequence>(buffers, beg, len);
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_SUB_BUFFERS_H_
