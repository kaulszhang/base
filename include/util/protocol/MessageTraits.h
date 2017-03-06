// MessageTraits.h

#ifndef _UTIL_PROTOCOL_MESSAGE_TRAITS_H_
#define _UTIL_PROTOCOL_MESSAGE_TRAITS_H_

namespace util
{
    namespace protocol
    {

        template <
            typename MsgT
        >
        class MessageHelper;

        template <
            typename MsgT
        >
        class MessageDataHelper;

        class MessageTraits
        {
        public:
            typedef boost::uint32_t id_type; // 可以重新定义

            typedef void header_type; // 必须重新定义

            typedef void i_archive_t; // 必须重新定义

            typedef void o_archive_t; // 必须重新定义

            static size_t const max_size = 0; // 必须重新定义

            typedef void context_t;

            typedef MessageHelper<MessageTraits> helper_t;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_TRAITS_H_
