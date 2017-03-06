// SnapshotBuffer.h

#ifndef _UTIL_BUFFERS_SNAPSHOT_BUFFER_H_
#define _UTIL_BUFFERS_SNAPSHOT_BUFFER_H_

#include <boost/asio/buffer.hpp>

#include <string.h>

namespace util
{
    namespace buffers
    {
        class SnapshotBuffer;
    }
}

namespace boost
{
    namespace asio
    {
        namespace detail
        {
            void const * buffer_cast_helper(
                util::buffers::SnapshotBuffer const & b);

            std::size_t buffer_size_helper(
                util::buffers::SnapshotBuffer const & b);

        }
    }
}

namespace util
{
    namespace buffers
    {

        class SnapshotBuffer
        {
        public:
            SnapshotBuffer()
                : data_(NULL)
                , size_(0)
                , own_(false)
            {
            }

            SnapshotBuffer(
                void const * data, 
                std::size_t size)
                : data_(data)
                , size_(size)
                , own_(false)
            {
            }

            template <typename ConstBufferSequence>
            SnapshotBuffer(
                ConstBufferSequence buffers, 
                std::size_t size)
                : data_(NULL)
                , size_(0)
                , own_(false)
            {
                typedef typename ConstBufferSequence::const_iterator iterator;

                using namespace boost::asio;

                for (iterator iter = buffers.begin(); iter != buffers.end(); ++ iter) {
                    if (data_ == NULL) { // first
                        if (buffer_size(*iter) >= size) {
                            *this = buffer(*iter);
                            break;
                        } else {
                            data_ = new char[size];
                            size_ = size;
                            own_ = true;
                            memcpy((char *)data_, buffer_cast<void const *>(*iter), buffer_size(*iter));
                            size -= buffer_size(*iter);
                        }
                    } else {
                        if (buffer_size(*iter) >= size) {
                            memcpy((char *)data_ + size_ - size, buffer_cast<void const *>(*iter), size);
                            break;
                        } else {
                            memcpy((char *)data_ + size_ - size, buffer_cast<void const *>(*iter), buffer_size(*iter));
                            size -= buffer_size(*iter);
                        }
                    }
                }
            }

            template <typename ConstBufferSequence>
            SnapshotBuffer(
                ConstBufferSequence buffers, 
                std::size_t offset, 
                std::size_t size)
                : data_(NULL)
                , size_(0)
                , own_(false)
            {
                typedef typename ConstBufferSequence::const_iterator iterator;

                using namespace boost::asio;

                for (iterator iter = buffers.begin(); iter != buffers.end(); ++ iter) {
                    const_buffer buf = buffer(*iter);
                    if (offset) {
                        if (offset < buffer_size(buf)) {
                            offset = 0;
                            buf = buf + offset;
                        } else {
                            offset -= buffer_size(buf);
                            continue;
                        }
                    }
                    if (data_ == NULL) { // first
                        if (buffer_size(buf) >= size) {
                            *this = buf;
                            break;
                        } else {
                            data_ = new char[size];
                            size_ = size;
                            own_ = true;
                            memcpy((char *)data_, buffer_cast<void const *>(buf), buffer_size(buf));
                            size -= buffer_size(buf);
                        }
                    } else {
                        if (buffer_size(buf) >= size) {
                            memcpy((char *)data_ + size_ - size, buffer_cast<void const *>(buf), size);
                            break;
                        } else {
                            memcpy((char *)data_ + size_ - size, buffer_cast<void const *>(buf), buffer_size(buf));
                            size -= buffer_size(buf);
                        }
                    }
                }
            }

            SnapshotBuffer(
                SnapshotBuffer const & other)
                : data_(other.data_)
                , size_(other.size_)
                , own_(false)
            {
            }

            SnapshotBuffer(
                const boost::asio::const_buffer & b)
                : data_(boost::asio::detail::buffer_cast_helper(b))
                , size_(boost::asio::detail::buffer_size_helper(b))
                , own_(false)
            {
            }

            SnapshotBuffer(
                const boost::asio::mutable_buffer& b)
                : data_(boost::asio::detail::buffer_cast_helper(b))
                , size_(boost::asio::detail::buffer_size_helper(b))
                , own_(false)
            {
            }

            operator boost::asio::const_buffer() const
            {
                return boost::asio::const_buffer(data_, size_);
            }

            ~SnapshotBuffer()
            {
                if (own_)
                    delete [] (char *)data_;
            }

            friend void const * boost::asio::detail::buffer_cast_helper(
                const SnapshotBuffer & b);

            friend std::size_t boost::asio::detail::buffer_size_helper(
                const SnapshotBuffer & b);

        private:
            void const * data_;
            size_t size_;
            bool own_;
        };

    }
}

namespace boost
{
    namespace asio
    {
        namespace detail
        {
            inline void const * buffer_cast_helper(
                util::buffers::SnapshotBuffer const & b)
            {
                return b.data_;
            }

            inline std::size_t buffer_size_helper(
                util::buffers::SnapshotBuffer const & b)
            {
                return b.size_;
            }
        }

        template <typename PointerToPodType>
        inline PointerToPodType buffer_cast(
            util::buffers::SnapshotBuffer const & b)
        {
            return static_cast<PointerToPodType>(detail::buffer_cast_helper(b));
        }

        inline std::size_t buffer_size(
            util::buffers::SnapshotBuffer const & b)
        {
            return detail::buffer_size_helper(b);
        }

        inline const_buffers_1 buffer(
            const util::buffers::SnapshotBuffer& b)
        {
            return const_buffers_1(
                const_buffer(buffer_cast<void const *>(b), buffer_size(b)));
        }

        inline const_buffers_1 buffer(
            const util::buffers::SnapshotBuffer& b,
            std::size_t max_size_in_bytes)
        {
            return const_buffers_1(
                const_buffer(buffer_cast<void const *>(b),
                buffer_size(b) < max_size_in_bytes ? buffer_size(b) : max_size_in_bytes));
        }
    }
}

#endif // _UTIL_BUFFERS_SNAPSHOT_BUFFER_H_
