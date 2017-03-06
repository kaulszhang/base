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
            typedef boost::uint32_t id_type; // �������¶���

            typedef void header_type; // �������¶���

            typedef void i_archive_t; // �������¶���

            typedef void o_archive_t; // �������¶���

            static size_t const max_size = 0; // �������¶���

            typedef void context_t;

            typedef MessageHelper<MessageTraits> helper_t;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_TRAITS_H_
