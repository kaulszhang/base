// MessageBase.h

#ifndef _UTIL_PROTOCOL_MESSAGE_BASE_H_
#define _UTIL_PROTOCOL_MESSAGE_BASE_H_

#include "util/protocol/MessageDefine.h"

namespace util
{
    namespace protocol
    {

        class MessageBase
        {
        public:
            MessageBase();

            ~MessageBase();

        public:
            MessageBase(
                MessageBase const & r);

            MessageBase & operator=(
                MessageBase const & r);

        public:
            bool empty() const;

        protected:
            void from_data(
                void * ar, 
                void *);

            void to_data(
                void * ar, 
                void *) const;

            void reset();

            void reset(
                MessageDefine const * def);

        protected:
            MessageDefine const * def_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_BASE_H_
