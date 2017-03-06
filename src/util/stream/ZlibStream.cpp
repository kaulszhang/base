// ZlibStream.cpp

#include "util/Util.h"
#include "util/stream/ZlibStream.h"
#include "zlib/zlib.h"

namespace util
{
    namespace stream
    {
        namespace helper
        {

            DataBuffer::DataBuffer()
                : m_buffer_is_local_(true)
                , m_buffer_(NULL)
                , m_buffer_size_(0)
                , m_data_size_(0)
            {
            }

            DataBuffer::DataBuffer(boost::uint32_t size)
                : m_buffer_is_local_(true)
                , m_buffer_(size ? new boost::uint8_t[size] : NULL)
                , m_buffer_size_(size)
                , m_data_size_(0)
            {
            }

            DataBuffer::DataBuffer(const void * data, boost::uint32_t size, bool copy)
                : m_buffer_is_local_(copy)
                , m_buffer_(copy ? (size ? new boost::uint8_t[size] : NULL) : reinterpret_cast<boost::uint8_t *>(const_cast<void *>(data)))
                , m_buffer_size_(size)
                , m_data_size_(size)
            {
                if (copy && size) memcpy(m_buffer_, data, size);
            }

            DataBuffer::DataBuffer(const DataBuffer & other)
                : m_buffer_is_local_(true)
                , m_buffer_(NULL)
                , m_buffer_size_(other.m_data_size_)
            {
                if (m_buffer_size_) {
                    m_buffer_ = new boost::uint8_t[m_buffer_size_];
                    memcpy(m_buffer_, other.m_buffer_, m_buffer_size_);
                }
            }

            DataBuffer::~DataBuffer()
            {
                clear();
            }

            boost::system::error_code DataBuffer::clear()
            {
                if (m_buffer_is_local_) {
                    delete[] m_buffer_;
                }

                m_buffer_ = NULL;
                m_data_size_ = 0;
                m_buffer_size_ = 0;

                return error::compress_success;
            }

            DataBuffer & DataBuffer::operator =(const DataBuffer & copy)
            {
                if (this != &copy) {
                    clear();

                    m_buffer_is_local_ = true;
                    m_buffer_size_ = copy.m_buffer_size_;
                    m_data_size_ = copy.m_data_size_;

                    if (m_buffer_size_) {
                        m_buffer_ = new boost::uint8_t[m_buffer_size_];
                        memcpy(m_buffer_, copy.m_buffer_, m_buffer_size_);
                    }
                }

                return *this;
            }

            bool DataBuffer::operator ==(const DataBuffer & other) const
            {
                if (m_data_size_ != other.m_data_size_) return false;

                return memcmp(m_buffer_, other.m_buffer_, m_data_size_) == 0;
            }

            boost::system::error_code DataBuffer::set_buffer(boost::uint8_t * buffer, boost::uint32_t buffer_size)
            {
                clear();
                m_buffer_is_local_ = false;
                m_buffer_ = buffer;
                m_buffer_size_ = buffer_size;
                m_data_size_ = 0;

                return error::compress_success;
            }

            boost::system::error_code DataBuffer::set_buffer_size(boost::uint32_t size)
            {
                if (m_buffer_is_local_) {
                    return _reallocate_buffer(size);
                } else {
                    return error::compress_not_supported;
                }
            }

            boost::system::error_code DataBuffer::reserve(boost::uint32_t size)
            {
                if (size <= m_buffer_size_) return error::compress_success;

                boost::uint32_t new_size = m_buffer_size_ * 2;
                if (new_size < size) new_size = size;

                return set_buffer_size(new_size);
            }

            boost::system::error_code DataBuffer::set_data_size(boost::uint32_t size)
            {
                if (size > m_buffer_size_) {
                    if (m_buffer_is_local_) {
                        boost::system::error_code ec = _reallocate_buffer(size);
                        if(ec) return ec;
                    } else {
                        return error::compress_not_supported;
                    }
                }

                m_data_size_ = size;

                return error::compress_success;
            }

            boost::system::error_code DataBuffer::set_data(const boost::uint8_t * data, boost::uint32_t data_size)
            {
                if (data_size > m_buffer_size_) {
                    if (m_buffer_is_local_) {
                        boost::system::error_code ec = _reallocate_buffer(data_size);
                        if(ec) return ec;
                    } else {
                        return error::compress_not_supported;
                    }
                }

                if (data) memcpy(m_buffer_, data, data_size);
                m_data_size_ = data_size;

                return error::compress_success;
            }

            boost::system::error_code DataBuffer::_reallocate_buffer(boost::uint32_t size)
            {
                if (m_data_size_ > size) return error::compress_invalid_parameters;

                boost::uint8_t * new_buffer = new boost::uint8_t[size];

                if (m_buffer_ && m_data_size_) {
                    memcpy(new_buffer, m_buffer_, m_data_size_);
                }

                delete[] m_buffer_;
                m_buffer_ = new_buffer;
                m_buffer_size_ = size;

                return error::compress_success;
            }

            MemoryStream::MemoryStream(const void * data, boost::uint32_t size)
                : m_buffer_((boost::uint8_t *)data, size)
                , m_read_offset_(0)
                , m_write_offset_(0)
            {
            }

            boost::system::error_code MemoryStream::read(void * buffer,
                boost::uint32_t bytes_to_read,
                boost::uint32_t * bytes_read/* = NULL*/)
            {
                if (bytes_to_read == 0) {
                    if (bytes_read) *bytes_read = 0;
                    return error::compress_success;
                }
                boost::uint32_t available = m_buffer_.get_data_size();
                if (m_read_offset_ + bytes_to_read > available) {
                    bytes_to_read = available - m_read_offset_;
                }

                if (bytes_to_read) {
                    memcpy(buffer, (void *)(((char *)m_buffer_.use_data()) + m_read_offset_), bytes_to_read);
                    m_read_offset_ += bytes_to_read;
                }
                if (bytes_to_read) {
                    *bytes_read = bytes_to_read;
                }

                return bytes_to_read ? error::compress_success : error::compress_eos;
            }

            boost::system::error_code MemoryStream::_input_seek(boost::uint64_t offset)
            {
                if (offset > m_buffer_.get_data_size()) {
                    return error::compress_out_of_range;
                } else {
                    m_read_offset_ = (boost::uint32_t)offset;
                    return error::compress_success;
                }
            }

            boost::system::error_code MemoryStream::write(const void * buffer, boost::uint32_t bytes_to_write, boost::uint32_t * bytes_written /* = NULL */)
            {
                boost::system::error_code ec = m_buffer_.reserve(m_write_offset_ + bytes_to_write);
                if (ec) return ec;

                memcpy(m_buffer_.use_data() + m_write_offset_, buffer, bytes_to_write);
                m_write_offset_ += bytes_to_write;
                if (m_write_offset_ > m_buffer_.get_data_size()) {
                    m_buffer_.set_data_size(m_write_offset_);
                }
                if (bytes_written) *bytes_written = bytes_to_write;

                return error::compress_success;
            }

            boost::system::error_code MemoryStream::_output_seek(boost::uint64_t offset)
            {
                if (offset <= m_buffer_.get_data_size()) {
                    m_write_offset_ = (boost::uint32_t)offset;
                    return error::compress_success;
                }

                return error::compress_out_of_range;
            }

            boost::system::error_code MemoryStream::set_data_size(boost::uint32_t size)
            {
                boost::system::error_code ec = m_buffer_.set_data_size(size);
                return ec;

                if (m_read_offset_ > size) m_read_offset_ = size;
                if (m_write_offset_ > size) m_write_offset_ = size;

                return error::compress_success;
            }
        }

        const unsigned int ZIP_DEFAULT_BUFFER_SIZE = 4096;

        static boost::system::error_code map_error(int err)
        {
            switch (err)
            {
            case Z_OK:            return error::compress_success;
            case Z_STREAM_END:    return error::compress_eos;
            case Z_DATA_ERROR:
            case Z_STREAM_ERROR:  return error::compress_invalid_format;
            case Z_MEM_ERROR:     return error::compress_out_of_memory;
            case Z_VERSION_ERROR: return error::compress_internal;
            case Z_NEED_DICT:     return error::compress_not_supported;
            default:              return error::compress_failure;
            }
        }

        class ZipInflateState
        {
        public:
            ZipInflateState() {
                memset(&m_stream_, 0, sizeof(m_stream_));
                inflateInit2(&m_stream_, 15 + 32); // // 15 = default window bits, +32 = automatic header
            }

            ~ZipInflateState() {
                inflateEnd(&m_stream_);
            }

            z_stream m_stream_;
        };

        ZipInflatingInputStream::ZipInflatingInputStream(helper::InputStreamReference & source)
            : m_source_(source)
            , m_position_(0)
            , m_state_(new ZipInflateState())
            , m_buffer_(ZIP_DEFAULT_BUFFER_SIZE) {
        }

        ZipInflatingInputStream::ZipInflatingInputStream(boost::asio::streambuf & compressed_buffer)
            : m_source_(helper::InputStreamReference(
            new helper::MemoryStream(
            (const void *)boost::asio::detail::buffer_cast_helper(compressed_buffer.data()),
            (boost::uint32_t)boost::asio::detail::buffer_size_helper(compressed_buffer.data()))))
            , m_position_(0)
            , m_buffer_(ZIP_DEFAULT_BUFFER_SIZE)
        {
        }

        ZipInflatingInputStream::~ZipInflatingInputStream()
        {
            delete m_state_;
            m_state_ = NULL;
        }

        boost::system::error_code ZipInflatingInputStream::set_read_source(boost::asio::streambuf & compressed_buffer)
        {
            m_source_ = helper::InputStreamReference(
                new helper::MemoryStream(
                (const void *)boost::asio::detail::buffer_cast_helper(compressed_buffer.data()),
                (boost::uint32_t)boost::asio::detail::buffer_size_helper(compressed_buffer.data())));
            return error::compress_success;
        }

        boost::system::error_code ZipInflatingInputStream::set_read_source(helper::InputStreamReference & source)
        {
            m_source_ = source;
            return error::compress_success;
        }

        boost::system::error_code ZipInflatingInputStream::read(boost::asio::streambuf & uncompressed_buffer)
        {
            helper::DataBuffer buffer, outer_buffer;
            boost::system::error_code ec;
            bool expect_eos = false;
            boost::uint64_t uncompress_len = 0;
            std::ostream os(&uncompressed_buffer);
            for (;;) {
                boost::uint32_t chunk = ZIP_DEFAULT_BUFFER_SIZE * 2;
                buffer.set_data_size(chunk);
                boost::uint32_t bytes_read = 0;
                ec = read(buffer.use_data(), chunk, &bytes_read);
                if (expect_eos) {
                    assert(ec == error::compress_eos);
                    break;
                }
                if (ec == error::compress_eos) {
                } else {
                    assert(ec == error::compress_success);
                    os.write((const char *)buffer.get_data(), bytes_read);
                    //uncompressed_buffer.prepare(bytes_read);
                    //memcpy((void *)((char *)boost::asio::detail::buffer_cast_helper(uncompressed_buffer.data()) + uncompress_len), buffer.get_data(), bytes_read);
                    //uncompressed_buffer.commit(bytes_read);
                }
                assert(bytes_read <= chunk);
                if (bytes_read != chunk) expect_eos = true;
                uncompress_len += bytes_read;
            }

            return error::compress_success;
        }

        boost::system::error_code ZipInflatingInputStream::read(void * buffer,
            boost::uint32_t bytes_to_read,
            boost::uint32_t * bytes_read /*= NULL*/)
        {
            if (m_state_ == NULL) return error::compress_invalid_state;
            if (buffer == NULL) return error::compress_invalid_parameters;
            if (bytes_to_read == 0) return error::compress_success;

            if (bytes_read) *bytes_read = 0;

            m_state_->m_stream_.next_out = (Bytef *)buffer;
            m_state_->m_stream_.avail_out = (uInt)bytes_to_read;

            while (m_state_->m_stream_.avail_out) {
                int err = inflate(&m_state_->m_stream_, Z_NO_FLUSH);
                if (err == Z_STREAM_END) {
                    break;
                } else if (err == Z_OK) {
                    continue;
                } else if (err == Z_BUF_ERROR) {
                    boost::uint32_t input_bytes_read = 0;
                    boost::uint64_t source_buffer_size = 0;
                    boost::system::error_code ec = m_source_->get_size(source_buffer_size);
                    if (!ec) {
                        m_buffer_.set_buffer_size(source_buffer_size);
                    } else {
                        m_buffer_.set_buffer_size(ZIP_DEFAULT_BUFFER_SIZE);
                    }
                    boost::system::error_code result = m_source_->read(m_buffer_.use_data(), m_buffer_.get_buffer_size(), &input_bytes_read);
                    if (result) break;//return result;

                    m_buffer_.set_data_size(input_bytes_read);
                    m_state_->m_stream_.next_in = m_buffer_.use_data();
                    m_state_->m_stream_.avail_in = m_buffer_.get_data_size();
                } else {
                    return map_error(err);
                }
            }

            boost::uint32_t progress = bytes_to_read - m_state_->m_stream_.avail_out;
            if (bytes_read) *bytes_read = progress;
            m_position_ += progress;

            return progress == 0 ? error::compress_eos : error::compress_success;
        }

        boost::system::error_code ZipInflatingInputStream::seek(boost::uint64_t /*offset*/)
        {
            return error::compress_not_supported;
        }

        boost::system::error_code ZipInflatingInputStream::tell(boost::uint64_t & offset)
        {
            offset = m_position_;
            return error::compress_success;
        }

        boost::system::error_code ZipInflatingInputStream::get_size(boost::uint64_t & size)
        {
            size = 0;
            return error::compress_not_supported;
        }

        boost::system::error_code ZipInflatingInputStream::get_available(boost::uint64_t & available)
        {
            available = 0;
            return error::compress_success;
        }
    }
}
