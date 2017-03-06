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
            size_t step_; // �Ѿ���ɵĲ���
            size_t size_; // ��һ�����������Ҫ���ֽ��������ֽ�����
            MessageDefine const * msg_def_; // ������ɵĽ������Ϣ�Ķ���
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_TRAITS_H_
