// ZlibStream.h

#ifndef _UTIL_STREAM_ZLIBSTREAM_H_
#define _UTIL_STREAM_ZLIBSTREAM_H_

#include <string>

#include <boost/config.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/error.hpp>
#include <boost/shared_ptr.hpp>

#include "util/stream/ZipError.h"

namespace util
{
    namespace stream
    {
        namespace helper
        {

            class DataBuffer 
            {
            public:
                DataBuffer();
                DataBuffer(boost::uint32_t size);
                DataBuffer(const void * data, boost::uint32_t size, bool copy = false);
                DataBuffer(const DataBuffer & other);
                ~DataBuffer();

                DataBuffer & operator =(const DataBuffer & copy);
                bool         operator==(const DataBuffer & other) const;

                virtual boost::system::error_code   set_buffer(boost::uint8_t * buffer, boost::uint32_t buffer_size);
                virtual boost::system::error_code   set_buffer_size(boost::uint32_t size);
                virtual boost::uint32_t             get_buffer_size() const { return m_buffer_size_; }
                virtual boost::system::error_code   reserve(boost::uint32_t size);
                virtual boost::system::error_code   clear();

                virtual const boost::uint8_t *      get_data() const { return m_buffer_; }
                virtual boost::uint8_t *            use_data() { return m_buffer_; }
                virtual boost::uint32_t             get_data_size() const { return m_data_size_; }
                virtual boost::system::error_code   set_data_size(boost::uint32_t size);
                virtual boost::system::error_code   set_data(const boost::uint8_t * data, boost::uint32_t data_size);

            protected:
                bool              m_buffer_is_local_;
                boost::uint8_t *  m_buffer_;
                boost::uint32_t   m_buffer_size_;
                boost::uint32_t   m_data_size_;

                boost::system::error_code _reallocate_buffer(boost::uint32_t size);
            };

            class InputStream
            {
            public:
                virtual ~InputStream() {}

                virtual boost::system::error_code read(void * buffer,
                    boost::uint32_t bytes_to_read,
                    boost::uint32_t * bytes_read = NULL) = 0;
                virtual boost::system::error_code seek(boost::uint64_t offset) = 0;
                virtual boost::system::error_code tell(boost::uint64_t & offset) = 0;
                virtual boost::system::error_code get_size(boost::uint64_t & size) = 0;
                virtual boost::system::error_code get_available(boost::uint64_t & available) = 0;
            };
            typedef boost::shared_ptr<InputStream> InputStreamReference;

            class OutputStream
            {
            public:
                virtual ~OutputStream() {}

                virtual boost::system::error_code write(const void * buffer,
                    boost::uint32_t bytes_to_write,
                    boost::uint32_t * bytes_written = NULL) = 0;
                virtual boost::system::error_code seek(boost::uint64_t offset) = 0;
                virtual boost::system::error_code tell(boost::uint64_t & offset) = 0;
            };

            typedef boost::shared_ptr<OutputStream> OutputStreamReference;

            class DelegatingInputStream : public InputStream
            {
            public:
                boost::system::error_code seek(boost::uint64_t offset)
                {
                    return _input_seek(offset);
                }
                boost::system::error_code tell(boost::uint64_t & offset)
                {
                    return _input_tell(offset);
                }

            private:
                virtual boost::system::error_code _input_seek(boost::uint64_t offset) = 0;
                virtual boost::system::error_code _input_tell(boost::uint64_t & offset) = 0;
            };

            class DelegatingOutputStream : public OutputStream
            {
            public:
                boost::system::error_code seek(boost::uint64_t offset)
                {
                    return _output_seek(offset);
                }
                boost::system::error_code tell(boost::uint64_t & offset)
                {
                    return _output_tell(offset);
                }

            private:
                virtual boost::system::error_code _output_seek(boost::uint64_t offset) = 0;
                virtual boost::system::error_code _output_tell(boost::uint64_t & offset) = 0;
            };

            class MemoryStream : public DelegatingInputStream, public DelegatingOutputStream
            {
            public:
                MemoryStream(const void * data, boost::uint32_t size);
                virtual ~MemoryStream() {}

                const DataBuffer & get_buffer() const { return m_buffer_; }

                boost::system::error_code read(void * buffer,
                    boost::uint32_t bytes_to_read,
                    boost::uint32_t * bytes_read = NULL);

                boost::system::error_code get_size(boost::uint64_t & size) {
                    size = m_buffer_.get_data_size();
                    return error::compress_success;
                }

                boost::system::error_code get_available(boost::uint64_t & available) {
                    available = (boost::uint64_t)m_buffer_.get_data_size() - m_read_offset_;
                    return error::compress_success;
                }

                boost::system::error_code write(const void * buffer,
                    boost::uint32_t bytes_to_write,
                    boost::uint32_t * bytes_written = NULL);

                const boost::uint8_t * get_data() const {
                    return m_buffer_.get_data();
                }

                boost::uint8_t * use_data() {
                    return m_buffer_.use_data();
                }

                boost::uint32_t get_data_size() const {
                    return m_buffer_.get_data_size();
                }

                boost::uint32_t get_buffer_size() const {
                    return m_buffer_.get_buffer_size();
                }

                boost::system::error_code set_data_size(boost::uint32_t size);

            private:
                boost::system::error_code _input_seek(boost::uint64_t offset);
                boost::system::error_code _input_tell(boost::uint64_t & offset) {
                    offset = m_read_offset_;
                    return error::compress_success;
                }

                boost::system::error_code _output_seek(boost::uint64_t offset);
                boost::system::error_code _output_tell(boost::uint64_t & offset) {
                    offset = m_write_offset_;
                    return error::compress_success;
                }

            protected:
                DataBuffer m_buffer_;
                boost::uint32_t m_read_offset_;
                boost::uint32_t m_write_offset_;
            };

            typedef boost::shared_ptr<MemoryStream> MemoryStreamReference;

        }

        class ZipInflateState;
        class ZipInflatingInputStream : public helper::InputStream
        {
        public:
            ZipInflatingInputStream(helper::InputStreamReference & source);
            ZipInflatingInputStream(boost::asio::streambuf & compressed_buffer);
            ~ZipInflatingInputStream();

            boost::system::error_code set_read_source(boost::asio::streambuf & compressed_buffer);
            boost::system::error_code set_read_source(helper::InputStreamReference & source);

            boost::system::error_code read(boost::asio::streambuf & uncompressed_buffer);

            virtual boost::system::error_code read(void * buffer,
                boost::uint32_t bytes_to_read,
                boost::uint32_t * bytes_read = NULL);
            virtual boost::system::error_code seek(boost::uint64_t offset);
            virtual boost::system::error_code tell(boost::uint64_t & offset);
            virtual boost::system::error_code get_size(boost::uint64_t & size);
            virtual boost::system::error_code get_available(boost::uint64_t & available);

        private:
            helper::InputStreamReference    m_source_;
            boost::uint64_t                 m_position_;
            ZipInflateState *               m_state_;
            helper::DataBuffer              m_buffer_;
        };
    }
}

#endif // _UTIL_STREAM_ZLIBSTREAM_H_
