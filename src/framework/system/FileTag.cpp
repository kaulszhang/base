// FileTag.cpp

#include "framework/Framework.h"
#include "framework/system/FileTag.h"
#include "framework/system/ErrorCode.h"

#include <boost/regex.hpp>
#include <boost/filesystem/operations.hpp>

#include <string.h>

namespace framework
{
    namespace system
    {

        FileFinder::FileFinder(
            std::string const & file)
            : fs_(file.c_str(), std::ios::in | std::ios::out | std::ios::binary)
            , pos_(0)
        {
        }

        bool FileFinder::find(
            std::string const & regex, 
            std::string & result)
        {
            boost::regex expr(regex);
            while (!fs_.eof()) {
                boost::asio::mutable_buffer mbuf = buf_.prepare(4096 - buf_.size());
                fs_.read(boost::asio::buffer_cast<char *>(mbuf), boost::asio::buffer_size(mbuf));
                buf_.commit(fs_.gcount());
                boost::asio::const_buffer cbuf = buf_.data();
                char const * buf_head = (char const *)boost::asio::buffer_cast<char const *>(cbuf);
                size_t buf_size = boost::asio::buffer_size(cbuf);
                boost::match_results<char const *> match_results;
                if (boost::regex_search(buf_head, buf_head + buf_.size(), match_results, expr,
                    boost::match_default | boost::match_partial)) {
                        if (match_results[0].matched) {
                            // Full match. We're done.
                            result = match_results[0].str();
                            size_t buf_pos = match_results[0].second - buf_head;
                            pos_ += buf_pos;
                            buf_.consume(buf_pos);
                            return true;
                        } else {
                            // Partial match. Next search needs to start from beginning of match.
                            size_t buf_pos = match_results[0].first - buf_head;
                            pos_ += buf_pos;
                            buf_.consume(buf_pos);
                        }
                } else {
                    // No match. Next search can start with the new data.
                    pos_ += buf_size;
                    buf_.consume(buf_size);
                }
            }
            return false;
        }

        void FileFinder::read(
            std::string & result)
        {
            size_t size = result.size();
            if (buf_.size() < size) {
                boost::asio::mutable_buffer mbuf = buf_.prepare(4096 - buf_.size());
                fs_.read(boost::asio::buffer_cast<char *>(mbuf), boost::asio::buffer_size(mbuf));
                buf_.commit(fs_.gcount());
            }
            if (buf_.size() < size) {
                size = buf_.size();
            }
            result.resize(size);
            memcpy(&result[0], boost::asio::buffer_cast<char const *>(buf_.data()), size);
            buf_.consume(size);
            pos_ += size;
        }

        void FileFinder::write(
            std::string const & result)
        {
            size_t size = result.size();
            fs_.seekp(pos_);
            fs_.write(&result[0], size);
            if (buf_.size() < size) {
                size = buf_.size();
            }
            buf_.consume(size);
            pos_ += size;
            fs_.seekg(pos_ + buf_.size());
        }

        bool FileFinder::fail() const
        {
            return fs_.fail();
        }

        void FileFinder::close()
        {
            return fs_.close();
        }
 
        FileTag::FileTag(
            std::string const & file, 
            std::string const & tag)
            : file_(file)
            , tag_(tag)
        {
        }

        boost::system::error_code FileTag::get(
            std::string const & item, 
            std::string & value)
        {
            FileFinder fs(file_);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            std::string tag = "!" + item + "\\|" + tag_ + "\\|tag";
            std::string result;
            if (!fs.find(tag, result)) {
                return framework::system::logic_error::item_not_exist;
            }
            value.resize(32);
            fs.read(value);
            value.resize(::strlen(value.c_str()));
            fs.close();

            return boost::system::error_code();
        }

        boost::system::error_code FileTag::set(
            std::string const & item, 
            std::string const & value)
        {
            FileFinder fs(file_);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            time_t last_write_time = boost::filesystem::last_write_time(file_);

            std::string tag = "!" + item + "\\|" + tag_ + "\\|tag";
            std::string result;
            if (!fs.find(tag, result)) {
                return framework::system::logic_error::item_not_exist;
            }
            fs.write(value + '\0'); // write terminate \0
            fs.close();

            boost::filesystem::last_write_time(file_, last_write_time);

            return boost::system::error_code();
        }

        boost::system::error_code FileTag::get_all(
            std::map<std::string, std::string> & values)
        {
            FileFinder fs(file_);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            bool all = values.empty();

            std::string tag = "!\\w+\\|" + tag_ + "\\|tag";
            std::string result;
            while (fs.find(tag, result)) {
                result = result.substr(1, result.find('|') - 1);
                std::map<std::string, std::string>::iterator iter = values.find(result);
                if (all && iter == values.end())
                    iter = values.insert(std::make_pair(result, "")).first;
                if (iter != values.end()) {
                    std::string value;
                    value.resize(32);
                    fs.read(value);
                    value.resize(::strlen(value.c_str()));
                    iter->second = value;
                }
            }

            fs.close();

            return boost::system::error_code();
        }

        boost::system::error_code FileTag::set_all(
            std::map<std::string, std::string> & values)
        {
            FileFinder fs(file_);
            if (fs.fail()) {
                return framework::system::last_system_error();
            }

            std::string tag = "!\\w+\\|" + tag_ + "\\|tag";
            std::string result;
            while (fs.find(tag, result)) {
                result = result.substr(1, result.find('|') - 1);
                printf("%s\n", result.c_str());
                std::map<std::string, std::string>::iterator iter = values.find(result);
                if (iter != values.end()) {
                    fs.write(iter->second + '\0'); // write terminate \0
                    iter->second.clear();
                }
            }

            fs.close();

            return boost::system::error_code();
        }

    } // namespace system
} // namespace boost
