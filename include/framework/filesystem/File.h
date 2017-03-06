// File.h

#ifndef _FRAMEWORK_FILE_SYSTEM_FILE_H_
#define _FRAMEWORK_FILE_SYSTEM_FILE_H_

#include "framework/container/Array.h"

#include <boost/filesystem/path.hpp>
#include <boost/asio/buffer.hpp>

namespace framework
{
    namespace filesystem
    {

        /* 简单的常规文件读写类
         * std::fstream 的局限性
         *   不支持一次性完成多段缓冲的读写
         * boost::asio::stream_discriptor 
         *   不支持常规文件
         * boost::asio::stream_handle
         *   需要外部打开句柄
         *   不支持seek
         * boost::asio::random_access_handle
         *   需要外部打开句柄
         *   需要外部记录读写位置
         */

        class File
        {
        public:
            enum {
                f_exclude = 1, 
                f_trunc = 2, 
                f_create = 4, 
                f_read = 8, 
                f_write = 16, 
                f_read_write = f_read | f_write,
            };

        public:
            File();

            ~File();

        public:
            bool open(
                std::string const & name, 
                boost::system::error_code & ec);

            bool open(
                boost::filesystem::path const & path, 
                boost::system::error_code & ec);

            bool open(
                std::string const & name, 
                int flags, 
                boost::system::error_code & ec);

            bool open(
                boost::filesystem::path const & path, 
                int flags, 
                boost::system::error_code & ec);

            bool assign(
#ifdef BOOST_WINDOWS_API
                HANDLE handle, 
#else
                int fd, 
#endif
                boost::system::error_code & ec);

            bool is_open() const;

#ifdef BOOST_WINDOWS_API
            HANDLE native() const { return handle_; }
#else
            int native() const { return fd_; }
#endif

            void swap(
                File & r);

            bool close(
                boost::system::error_code & ec);

        public:
            enum SeekDir
            {
#ifdef BOOST_WINDOWS_API
                beg = FILE_BEGIN, 
                cur = FILE_CURRENT, 
                end = FILE_END, 
#else
                beg = SEEK_SET, 
                cur = SEEK_CUR, 
                end = SEEK_END, 
#endif
            };

            bool seek(
                SeekDir dir, 
                boost::uint64_t offset, 
                boost::system::error_code & ec);

            boost::uint64_t tell(
                boost::system::error_code & ec);

			boost::uint64_t size(
				boost::system::error_code & ec);

        public:
            typedef boost::asio::mutable_buffer mutable_buffer_t;

            typedef framework::container::Array<mutable_buffer_t> mutable_buffers_t;

            typedef boost::asio::const_buffer const_buffer_t;

            typedef framework::container::Array<const_buffer_t> const_buffers_t;

            size_t read_some(
                mutable_buffer_t const & buffer, 
                boost::system::error_code & ec);

            size_t read_some(
                mutable_buffers_t const & buffers, 
                boost::system::error_code & ec);

            size_t write_some(
                const_buffer_t const & buffer, 
                boost::system::error_code & ec);

            size_t write_some(
                const_buffers_t const & buffers, 
                boost::system::error_code & ec);

        private:
#ifdef BOOST_WINDOWS_API
            HANDLE handle_;
#else
            int fd_;
#endif
        };

    } // namespace filesystem
} // namespace framework

#endif // _FRAMEWORK_FILE_SYSTEM_FILE_H_
