// LoggerRecord.h

#ifndef _FRAMEWORK_LOGGER_RECORD_H_
#define _FRAMEWORK_LOGGER_RECORD_H_

#include "framework/logger/Logger.h"

namespace framework
{
    namespace logger
    {

        class LoggerRecord
        {
        protected:
            typedef size_t (*format_message_type)(
                LoggerRecord const & rec, 
                char * msg, 
                size_t len);

            typedef void (*destroy_self_type)(
                LoggerRecord const & rec);

            LoggerRecord(
                format_message_type format_message, 
                destroy_self_type destroy_self)
                : format_message_(format_message)
                , destroy_self_(destroy_self)
            {
            }

            ~LoggerRecord()
            {
            }

        public:
            size_t const format_message(
                char * msg, 
                size_t len) const
            {
                return format_message_(*this, msg, len);
            }

            void destroy() const
            {
                destroy_self_(*this);
            }

        private:
            format_message_type format_message_;
            destroy_self_type destroy_self_;
        };

    } // namespace logger
} // namespace framework

#endif // _FRAMEWORK_LOGGER_RECORD_H_
