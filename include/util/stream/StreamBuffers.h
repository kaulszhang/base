// StreamBuffers.h

#ifndef _UTIL_STREAM_STREAM_BUFFERS_H_
#define _UTIL_STREAM_STREAM_BUFFERS_H_

#include <framework/Framework.h>

#include <boost/asio/buffer.hpp>
#include <boost/intrusive_ptr.hpp>

#include <utility>

namespace util
{
    namespace stream
    {

        template <
            typename Buffer
        >
        struct Buf;

        template <
            typename Buffer
        >
        void intrusive_ptr_add_ref(
            Buf<Buffer> * p);

        template <
            typename Buffer
        >
        void intrusive_ptr_release(
            Buf<Buffer>* p);

        template <
            typename Buffer
        >
        struct Buf
            : std::STREAMSDK_CONTANINER<Buffer>
        {
            Buf()
                : nref(0)
            {
            }

            Buf(
                Buf const & r)
                : std::STREAMSDK_CONTANINER<Buffer>(r)
                , nref(0)
            {
            }

            size_t nref;

            template <typename B>
            friend void intrusive_ptr_add_ref(
                Buf<B> * p);
    
            template <typename B>
            friend void intrusive_ptr_release(
                Buf<B> * p);
        };

        template <
            typename Buffer
        >
        inline void intrusive_ptr_add_ref(
            Buf<Buffer> * p)
        {
            ++p->nref;
        }

        template <
            typename Buffer
        >
        inline void intrusive_ptr_release(
            Buf<Buffer> * p)
        {
            if (--p->nref == 0) {
                delete p;
            }
        }

        template <
            typename Buffer
        >
        class StreamBuffers
        {
        public:
            typedef Buffer value_type;
            typedef typename std::STREAMSDK_CONTANINER<Buffer>::const_iterator const_iterator;
            typedef Buf<Buffer> buf_t;

        public:
            StreamBuffers()
            {
            }

            template <
                typename BufferSequence
            >
            StreamBuffers(
                BufferSequence const & buffers)
            {
                typename BufferSequence::const_iterator beg = buffers.begin();
                typename BufferSequence::const_iterator end = buffers.end();
                size_t count = std::distance(beg, end);
                if (count > 0) {
                    buf_.reset(new buf_t);
                    std::copy(beg, end, std::back_inserter(*buf_));
                }
            }

        public:
            const_iterator begin() const
            {
                return buf_ ? buf_->begin() : const_iterator();
            }

            const_iterator end() const
            {
                return buf_ ? buf_->end(): const_iterator();
            }

            size_t size() const
            {
                return buf_ ? buf_->size() : 0;
            }

        public:
            void reset()
            {
                buf_.reset();
            }

            void reset(
                StreamBuffers const & r)
            {
                buf_.reset(r.buf_);
            }

            StreamBuffers & operator=(
                StreamBuffers const & r)
            {
                reset(r);
                return *this;
            }

        public:
            void push_front(
                Buffer const & buffer)
            {
                before_modify();
                buf_->push_front(buffer);
            }

            void push_back(
                Buffer const & buffer)
            {
                before_modify();
                buf_->push_back(buffer);
            }

            void pop_front()
            {
                before_modify();
                buf_->pop_front();
            }

            void pop_back(
                size_t n)
            {
                before_modify();
                buf_->pop_back();
            }

            void clear()
            {
                if (buf_) {
                    if (buf_->nref > 1) {
                        reset();
                    } else {
                        buf_->clear();
                    }
                }
            }

        private:
            void before_modify()
            {
                if (!buf_) {
                    buf_.reset(new buf_t);
                } else if (buf_->nref > 1) {
                    buf_.reset(new buf_t(*buf_));
                }
            }

        private:
            boost::intrusive_ptr<buf_t> buf_;
        };

        typedef StreamBuffers<boost::asio::const_buffer> StreamConstBuffers;
        typedef StreamBuffers<boost::asio::mutable_buffer> StreamMutableBuffers;

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_SOURCE_H_
