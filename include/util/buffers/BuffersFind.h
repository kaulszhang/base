// BuffersFind.h

#ifndef _UTIL_BUFFERS_BUFFERS_FIND_H_
#define _UTIL_BUFFERS_BUFFERS_FIND_H_

#include "util/buffers/BuffersSize.h"
#include "util/buffers/SubBuffers.h"
#include "util/buffers/BuffersPosition.h"
#include "util/buffers/BuffersCompare.h"
#include "util/buffers/BuffersByteIterator.h"
#include "util/buffers/BuffersBufferIterator.h"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/asio/buffer.hpp>

namespace util
{
    namespace buffers
    {

        namespace detail
        {

            template <
                typename Buffer, 
                typename BufferIterator
            >
            void buffers_char(
                BuffersLimit<BufferIterator> const & limit, 
                BuffersPosition<Buffer, BufferIterator> & pos, 
                BuffersPosition<Buffer, BufferIterator> const & end, 
                char chr)
            {
                while (!pos.equal(end)) {
                    char const * buf = boost::asio::buffer_cast<char const *>(pos.dereference_buffer());
                    size_t size = boost::asio::buffer_size(pos.dereference_buffer());
                    char const * buf1 = (char const *)::memchr(buf, chr, size);
                    if (buf1) {
                        pos.increment_bytes(limit, end, buf1 - buf);
                        return;
                    }
                    pos.increment_buffer(limit, end);
                };
            }

        } // namespace detail

        template <
            typename Buffer, 
            typename BufferIterator
        >
        class BuffersCharIterator
            : public boost::iterator_facade<
                BuffersCharIterator<Buffer, BufferIterator>,
                BuffersPosition<Buffer, BufferIterator> const, 
                boost::forward_traversal_tag>
        {
        public:
            typedef BuffersLimit<BufferIterator> Limit;

            typedef BuffersPosition<Buffer, BufferIterator> Position;

            typedef BuffersBufferIterator<Buffer, BufferIterator> BuffersIterator;

            typedef BuffersByteIterator<Buffer, BufferIterator> ByteIterator;

        public:
            BuffersCharIterator(
                BufferIterator const & beg, 
                BufferIterator const & end, 
                int chr)
                : limit_(beg, end)
                , chr_(chr)
                , cur_(limit_, beg)
                , end_(limit_, end)
                , reset_postion_(false)
            {
                detail::buffers_char(limit_, cur_, end_, chr_);
            }

            BuffersCharIterator()
            {
            }
            
        public:
            template <
                typename Buffer2, 
                typename BuffersLimit2
            >
            int compare_continue(
                BuffersLimit2 const & limit2, 
                size_t size)
            {
                return detail::buffers_compare_impl<Buffer2>(limit_, limit2, cur_, end_, size);
            }

        public:
            void skip_bytes(
                size_t size)
            {
                cur_.increment_bytes(limit_, end_, size);
                reset_postion_ = true;
            }

            BufferIterator sub_buffers()
            {
                return BufferIterator(limit_, cur_, end_);
            }
            
            ByteIterator sub_bytes()
            {
                return ByteIterator(limit_, cur_, end_);
            }
            
            BufferIterator sub_buffers_from(
                Position const & pos)
            {
                return BufferIterator(limit_, pos, cur_);
            }
            
            ByteIterator sub_bytes_from(
                Position const & pos)
            {
                return ByteIterator(limit_, pos, cur_);
            }
            
            BufferIterator sub_buffers_to(
                Position const & pos)
            {
                return BufferIterator(limit_, cur_, pos);
            }
            
            ByteIterator sub_bytes_to(
                Position const & pos)
            {
                return ByteIterator(limit_, cur_, pos);
            }
            
            BufferIterator next_buffers()
            {
                Position pos = position();
                increment();
                return BufferIterator(limit_, pos, position());
            }

            ByteIterator next_bytes()
            {
                Position pos = position();
                increment();
                return ByteIterator(limit_, pos, position());
            }

            Position const & position() const
            {
                return cur_;
            }

            Position const & end_position() const
            {
                return end_;
            }

        protected:
            friend class boost::iterator_core_access;

            void increment()
            {
                if (!reset_postion_)
                    cur_.increment_byte(limit_);
                detail::buffers_char(limit_, cur_, end_, chr_);
                reset_postion_ = false;
            }

            bool equal(
                const BuffersCharIterator & other) const
            {
                return cur_.equal(other.cur_);
            }

            Position const & dereference() const
            {
                return cur_;
            }

        protected:
            Limit limit_;
            char chr_;
            Position cur_;
            Position end_;
            bool reset_postion_;
        };

        template <
            typename Buffer, 
            typename BufferIterator, 
            typename ConstBufferSequence2
        >
        class BuffersFindIterator
            : public boost::iterator_facade<
                BuffersFindIterator<Buffer, BufferIterator, ConstBufferSequence2>, 
                BuffersPosition<Buffer, BufferIterator> const, 
                boost::forward_traversal_tag>
        {
        public:
            typedef BuffersLimit<BufferIterator> Limit;

            typedef BuffersPosition<Buffer, BufferIterator> Position;

            typedef BuffersBufferIterator<Buffer, BufferIterator> BuffersIterator;

            typedef BuffersByteIterator<Buffer, BufferIterator> ByteIterator;

            typedef typename ConstBufferSequence2::value_type  Buffer2;

            typedef BuffersLimit<typename ConstBufferSequence2::const_iterator> Limit2;

            typedef BuffersCharIterator<Buffer, BufferIterator> CharIterator;

        public:
            BuffersFindIterator(
                BufferIterator const & beg, 
                BufferIterator const & end, 
                ConstBufferSequence2 const & find)
                : char_iter_(beg, end, first_char(find))
                , reset_postion_(false)
                , limit2_(find.begin(), find.end())
                , size_find_(buffers_size(find))
            {
                CharIterator end1;
                while (char_iter_ != end1 && char_iter_.template compare_continue<Buffer2>(limit2_, size_find_) != 0)
                    ++char_iter_;
            }

            BuffersFindIterator()
            {
            }

        public:
            void skip_bytes(
                size_t size)
            {
                char_iter_.skip_bytes(size);
                reset_postion_ = true;
            }

            BufferIterator sub_buffers()
            {
                return char_iter_.sub_buffers();
            }
            
            ByteIterator sub_bytes()
            {
                return char_iter_.sub_bytes();
            }
            
            BufferIterator next_buffers()
            {
                Position pos = position();
                increment();
               return char_iter_.sub_buffers_from(pos);
            }

            ByteIterator next_bytes()
            {
                Position pos = position();
                increment();
                return char_iter_.sub_bytes_from(pos);
            }

            Position const & position() const
            {
                return char_iter_.position();
            }

            Position const & end_position() const
            {
                return char_iter_.end_position();
            }

        private:
            static int first_char(
                ConstBufferSequence2 const & find)
            {
                typename ConstBufferSequence2::const_iterator iter = find.begin();
                typename ConstBufferSequence2::const_iterator end = find.end();
                while (iter != end) {
                    size_t n = boost::asio::buffer_size(buffer(*iter));
                    if (n > 0)
                        break;
                    ++iter;
                }
                int chr = *boost::asio::buffer_cast<char const *>(buffer(*iter));
                return chr;
            }
            
        private:
            friend class boost::iterator_core_access;

            void increment()
            {
                if (!reset_postion_)
                    char_iter_.skip_bytes(size_find_);
                ++char_iter_;
                CharIterator end;
                while (char_iter_ != end && char_iter_.template compare_continue<Buffer2>(limit2_, size_find_) != 0) {
                    ++char_iter_;
                }
                reset_postion_ = false;
            }

            bool equal(
                const BuffersFindIterator & other) const
            {
                return char_iter_ == other.char_iter_;
            }

            Position const & dereference() const
            {
                return *char_iter_;
            }

        private:
            CharIterator char_iter_;
            bool reset_postion_;
            Limit2 const limit2_;
            size_t size_find_;
        };

        template <
            typename ConstBufferSequence1, 
            typename ConstBufferSequence2
        >
        class BuffersFindIterator2
            : public BuffersFindIterator<
                typename ConstBufferSequence1::value_type, 
                typename ConstBufferSequence1::const_iterator, 
                ConstBufferSequence2>
        {
        public:
            typedef BuffersFindIterator<
                typename ConstBufferSequence1::value_type, 
                typename ConstBufferSequence1::const_iterator, 
                ConstBufferSequence2> super;

        public:
            BuffersFindIterator2()
            {
            }

            BuffersFindIterator2(
                ConstBufferSequence1 const & buffers, 
                ConstBufferSequence2 const & find)
                : super(buffers.begin(), buffers.end(), find)
            {
            }
        };

        template <typename ConstBufferSequence>
        BuffersPosition<
            typename ConstBufferSequence::value_type, 
            typename ConstBufferSequence::const_iterator
        > buffers_char(
            ConstBufferSequence const & buffers, 
            int chr)
        {
            typedef BuffersCharIterator<
                typename ConstBufferSequence::value_type, 
                typename ConstBufferSequence::const_iterator
            > Iterator;
            return Iterator(buffers.begin(), buffers.end(), chr).position();
        }

        template <typename ConstBufferSequence1, typename ConstBufferSequence2>
        BuffersPosition<
            typename ConstBufferSequence1::value_type, 
            typename ConstBufferSequence1::const_iterator
        > buffers_find(
            ConstBufferSequence1 const & buffers, 
            ConstBufferSequence2 const & find)
        {
            typedef BuffersFindIterator<
                typename ConstBufferSequence1::value_type, 
                typename ConstBufferSequence1::const_iterator, 
				ConstBufferSequence2
            > Iterator;
            return Iterator(buffers.begin(), buffers.end(), find).position();
        }

    } // namespace buffers
} // namespace util

#endif // _UTIL_BUFFERS_BUFFERS_FIND_H_
