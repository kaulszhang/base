// MessageParser.h

#ifndef _UTIL_PROTOCOL_MESSAGE_PARSER_H_
#define _UTIL_PROTOCOL_MESSAGE_PARSER_H_

#include <boost/asio/buffer.hpp>

namespace util
{
    namespace protocol
    {

        struct MessageDefine;

        class MessageParser
        {
        public:
            MessageParser()
                : ok_(false)
                , step_(0)
                , size_(0)
                , msg_def_(NULL)
            {
            }

        public:
            virtual void parse(
                boost::asio::const_buffer const & buf) = 0;

        public:
            bool ok() const
            {
                return ok_;
            }
            size_t size() const
            {
                return size_;
            }

            MessageDefine const * msg_def() const
            {
                return msg_def_;
            }

            void reset()
            {
                ok_ = false;
                step_ = 0;
                size_ = 0;
                msg_def_ = NULL;
                parse(boost::asio::const_buffer(this, 0));
            }

        protected:
            bool ok_;
            size_t step_; // 已经完成的步骤
            size_t size_; // 下一步完成至少需要的字节数（总字节数）
            MessageDefine const * msg_def_; // 解析完成的结果，消息的定义
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_TRAITS_H_
