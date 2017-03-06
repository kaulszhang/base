// MessageData.h

#ifndef _UTIL_PROTOCOL_MESSAGE_DATA_H_
#define _UTIL_PROTOCOL_MESSAGE_DATA_H_

#include "util/protocol/MessageTraits.h"
#include "util/protocol/Message.h"

namespace util
{
    namespace protocol
    {

        template <
            typename MsgT, 
            typename T, 
            typename MsgT::id_type id
        >
        class MessageData
        {
        public:
            typedef typename MsgT::id_type id_type;

            static id_type const static_id;

            static MessageDefine::ClassEnum const static_cls;

            static MessageDefineT<MsgT, T> const msg_def;
        };

        template <
            typename MsgT, 
            typename T, 
            typename MsgT::id_type id
        >
        typename MsgT::id_type const MessageData<MsgT, T, id>::static_id = id;

        template <
            typename MsgT, 
            typename T, 
            typename MsgT::id_type id
        >
        typename MessageDefine::ClassEnum const MessageData<MsgT, T, id>::static_cls = MessageDefine::control_message;

        template <
            typename MsgT, 
            typename T, 
            typename MsgT::id_type id
        >
        MessageDefineT<MsgT, T> const MessageData<MsgT, T, id>::msg_def(T::static_cls);

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_DATA_H_
