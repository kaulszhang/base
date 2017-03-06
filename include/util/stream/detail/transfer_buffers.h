// transfer_buffers.h

#ifndef _UTIL_STREAM_TRANSFER_BUFFERS_H_
#define _UTIL_STREAM_TRANSFER_BUFFERS_H_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/asio/buffer.hpp>

namespace util
{
    namespace stream
    {

        namespace detail
        {

            template <typename Buffer, typename Buffer_Iterator>
            struct buffers_status
            {
                bool at_end_;
                Buffer first_;
                Buffer_Iterator begin_remainder_;
            };

            template <typename BufferLeft, typename BufferRight, typename Buffer_Iterator>
            class transfer_buffers_iterator
                : public boost::iterator_facade<
                transfer_buffers_iterator<BufferLeft, BufferRight, Buffer_Iterator>,
                const BufferLeft, boost::forward_traversal_tag>
            {
            public:
                // Default constructor creates an end iterator.
                transfer_buffers_iterator()
                {
                    left_.at_end_ = true;
                }

                // Construct with a buffer for the first entry and an iterator
                // range for the remaining entries.
                transfer_buffers_iterator(
                    Buffer_Iterator begin, 
                    Buffer_Iterator end, 
                    buffers_status<BufferLeft, Buffer_Iterator> const & left,
                    buffers_status<BufferRight, Buffer_Iterator> const & right)
                    : begin_(begin)
                    , end_(end)
                    , left_(left)
                    , right_(right)
                    , offset_(0)
                {
                    using namespace boost::asio;
                    left_.first_ = buffer(left_.first_, max_size);
                }

            private:
                friend class boost::iterator_core_access;

                enum { max_size = 65536 };

                void increment()
                {
                    using namespace boost::asio;

                    if (!left_.at_end_)
                    {
                        offset_ += buffer_size(left_.first_);
                        if (offset_ >= max_size) {
                            left_.at_end_ = true;
                            return;
                        }
                        if (left_.begin_remainder_ == right_.begin_remainder_) {
                            if (buffer_cast<const char*>(right_.first_) < buffer_cast<const char*>(left_.first_) || 
                                (buffer_cast<const char*>(right_.first_) == buffer_cast<const char*>(left_.first_) && 
                                buffer_size(right_.first_) < buffer_size(left_.first_))) {
                                    // 左指针在右指针前面，还能循环，执行流程跳到下面
                            } else {
                                // 左指针在右指针后面，追上了，所以结束
                                left_.at_end_ = true;
                                return;
                            }
                        }
                        left_.first_ = buffer(*left_.begin_remainder_++, max_size - offset_);
                        if (left_.begin_remainder_ == end_)
                            left_.begin_remainder_ = begin_;
                        if (left_.begin_remainder_ == right_.begin_remainder_) {
                            size_t real_size = buffer_cast<const char*>(right_.first_) - buffer_cast<const char*>(left_.first_);
                            left_.first_ = buffer(left_.first_, real_size);
                            // 这最后一个，如果是空的话，舍去
                            if (buffer_size(left_.first_) == 0)
                                left_.at_end_ = true;
                        }
                    }
                }

                bool equal(const transfer_buffers_iterator& other) const
                {
                    using namespace boost::asio;

                    if (left_.at_end_ && other.left_.at_end_)
                        return true;
                    return !left_.at_end_ && !other.left_.at_end_
                        && buffer_cast<const void*>(left_.first_) == buffer_cast<const void*>(other.left_.first_)
                        && buffer_size(left_.first_) == buffer_size(other.left_.first_)
                        && left_.begin_remainder_ == other.left_.begin_remainder_;
                }

                const BufferLeft & dereference() const
                {
                    return left_.first_;
                }

                Buffer_Iterator begin_;
                Buffer_Iterator end_;
                buffers_status<BufferLeft, Buffer_Iterator> left_;
                buffers_status<BufferRight, Buffer_Iterator> right_;
                std::size_t offset_;
            };

            template <typename ConstBuffer, typename MutableBuffer, typename Buffers>
            class transfer_buffers
            {
            public:
                typedef ConstBuffer read_buffer_type;

                typedef MutableBuffer write_buffer_type;

            public:
                typedef ConstBuffer value_type;

                // A forward-only iterator type that may be used to read elements.
                typedef transfer_buffers_iterator<ConstBuffer, MutableBuffer, typename Buffers::const_iterator>
                    read_const_iterator;

                typedef transfer_buffers_iterator<MutableBuffer, ConstBuffer, typename Buffers::const_iterator>
                    write_const_iterator;

                class read_buffers
                {
                public:
                    // The type for each element in the list of buffers.
                    typedef read_buffer_type value_type;

                    // A forward-only iterator type that may be used to read elements.
                    typedef read_const_iterator const_iterator;

                    explicit read_buffers(
                        transfer_buffers const & buffers)
                        : buffers_(buffers)
                    {
                    }

                    // Get a forward-only iterator to the first element.
                    const_iterator begin() const
                    {
                        return buffers_.rbegin();
                    }

                    // Get a forward-only iterator for one past the last element.
                    const_iterator end() const
                    {
                        return buffers_.rend();
                    }

                private:
                    transfer_buffers const buffers_;
                };

                class write_buffers
                {
                public:
                    // The type for each element in the list of buffers.
                    typedef write_buffer_type value_type;

                    // A forward-only iterator type that may be used to read elements.
                    typedef write_const_iterator const_iterator;

                    explicit write_buffers(
                        transfer_buffers const & buffers)
                        : buffers_(buffers)
                    {
                    }

                    // Get a forward-only iterator to the first element.
                    const_iterator begin() const
                    {
                        return buffers_.wbegin();
                    }

                    // Get a forward-only iterator for one past the last element.
                    const_iterator end() const
                    {
                        return buffers_.wend();
                    }

                private:
                    transfer_buffers const buffers_;
                };

                typedef read_buffers read_buffers_type;
                typedef write_buffers write_buffers_type;

                write_buffers_type write_buffers() const
                {
                    return write_buffers_type(*this);
                }

                read_buffers_type read_buffers() const
                {
                    return read_buffers_type(*this);
                }

                // Construct to represent the entire list of buffers.
                transfer_buffers(
                    Buffers const & buffers)
                    : buffers_(buffers)
                {

                    write_.at_end_ = false;
                    write_.begin_remainder_ = buffers_.begin();
                    read_.at_end_ = true;
                    read_.begin_remainder_ = buffers_.begin();
                    if (buffers_.begin() == buffers_.end()) {
                        write_.at_end_ = true;
                    } else {
                        write_.first_ = *buffers_.begin();
                        ++write_.begin_remainder_;
                        if (write_.begin_remainder_ == buffers_.end())
                            write_.begin_remainder_ = buffers_.begin();
                        read_.first_ = boost::asio::buffer(*buffers_.begin(), 0);
                        ++read_.begin_remainder_;
                        if (read_.begin_remainder_ == buffers_.end())
                            read_.begin_remainder_ = buffers_.begin();
                    }
                }

                // Copy constructor.
                transfer_buffers(
                    const transfer_buffers & other)
                    : buffers_(other.buffers_)
                    , write_(other.write_)
                    , read_(other.read_)
                {
                    write_.begin_remainder_ = buffers_.begin();
                    read_.begin_remainder_ = buffers_.begin();
                    typename Buffers::const_iterator first = other.buffers_.begin();
                    typename Buffers::const_iterator second = other.write_.begin_remainder_;
                    std::advance(write_.begin_remainder_, std::distance(first, second));
                    typename Buffers::const_iterator third = other.read_.begin_remainder_;
                    std::advance(read_.begin_remainder_, std::distance(first, third));
                }

                // Assignment operator.
                transfer_buffers & operator=(const transfer_buffers& other)
                {
                    buffers_ = other.buffers_;
                    write_.at_end_ = other.write_.at_end_;
                    write_.first_ = other.write_.first_;
                    read_.at_end_ = other.read_.at_end_;
                    read_.first_ = other.read_.first_;
                    write_.begin_remainder_ = buffers_.begin();
                    read_.begin_remainder_ = buffers_.begin();
                    typename Buffers::const_iterator first = other.buffers_.begin();
                    typename Buffers::const_iterator second = other.write_.begin_remainder_;
                    std::advance(write_.begin_remainder_, std::distance(first, second));
                    typename Buffers::const_iterator third = other.read_.begin_remainder_;
                    std::advance(read_.begin_remainder_, std::distance(first, third));
                    return *this;
                }

                // Get a forward-only iterator to the first element.
                write_const_iterator wbegin() const
                {
                    return write_const_iterator(buffers_.begin(), buffers_.end(), write_, read_);
                }

                // Get a forward-only iterator for one past the last element.
                write_const_iterator wend() const
                {
                    return write_const_iterator();
                }

                // Get a forward-only iterator to the first element.
                read_const_iterator rbegin() const
                {
                    return read_const_iterator(buffers_.begin(), buffers_.end(), read_, write_);
                }

                // Get a forward-only iterator for one past the last element.
                read_const_iterator rend() const
                {
                    return read_const_iterator();
                }

                size_t total_buffer_size() const
                {
                    size_t total = 0;
                    typedef typename Buffers::const_iterator const_iterator;
                    for (const_iterator iter = buffers_.begin(); iter != buffers_.end(); ++iter) {
                        boost::asio::const_buffer buffer(*iter);
                        total += boost::asio::buffer_size(buffer);
                    }
                    return total;
                }

                template <typename BufferLeft, typename BufferRight>
                void shift(
                    BufferLeft & left, 
                    BufferRight & right, 
                    std::size_t size)
                {
                    using namespace boost::asio;
                    // Remove buffers from the start until the specified size is reached.
                    while (size > 0 && !left.at_end_)
                    {
                        right.at_end_ = false;
                        std::size_t this_size = buffer_size(left.first_);
                        if (this_size > size) {
                            this_size = size;
                        }
                        size -= this_size;
                        left.first_ = left.first_ + this_size;
                        if (right.begin_remainder_ == left.begin_remainder_) {
                            if (buffer_cast<const char*>(right.first_) < buffer_cast<const char*>(left.first_)) {
                                // 左指针在右指针前面，，左指针离开，右指针跟进
                                right.first_ = buffer(boost::asio::detail::buffer_cast_helper(right.first_), buffer_size(right.first_) + this_size);
                            } else if (buffer_size(left.first_) == 0) {
                                // 左指针在右指针后面，追上了，所以结束
                                left.at_end_ = true;
                                break;
                            }
                        }
                        if (buffer_size(left.first_) == 0) {
                            left.first_ = *left.begin_remainder_++;
                            if (left.begin_remainder_ == buffers_.end())
                                left.begin_remainder_ = buffers_.begin();
                            if (left.begin_remainder_ == right.begin_remainder_) {
                                // 左指针将要追上右指针，这一块数据只有一部分可访问
                                size_t real_size = buffer_cast<const char*>(right.first_) - buffer_cast<const char*>(left.first_);
                                left.first_ = buffer(left.first_, real_size);
                            }
                        }
                    }

                    // Remove any more empty buffers at the start.
                    while (!left.at_end_ && buffer_size(left.first_) == 0)
                    {
                        if (left.begin_remainder_ == right.begin_remainder_) {
                            left.at_end_ = true;
                        } else {
                            left.first_ = *left.begin_remainder_++;
                            if (left.begin_remainder_ == buffers_.end())
                                left.begin_remainder_ = buffers_.begin();
                            if (left.begin_remainder_ == right.begin_remainder_) {
                                // 左指针将要追上右指针，这一块数据只有一部分可访问
                                size_t real_size = buffer_cast<const char*>(right.first_) - buffer_cast<const char*>(left.first_);
                                left.first_ = buffer(left.first_, real_size);
                            }
                        }
                    }
                }

                // Consume the specified number of bytes from the buffers.
                void commit(std::size_t size)
                {
                    shift(write_, read_, size);
                }

                void consume(std::size_t size)
                {
                    shift(read_, write_, size);
                }

            private:
                Buffers buffers_;
                buffers_status<MutableBuffer, typename Buffers::const_iterator> write_;
                buffers_status<ConstBuffer, typename Buffers::const_iterator> read_;
            };

        } // namespace detail
    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_TRANSFER_BUFFERS_H_
