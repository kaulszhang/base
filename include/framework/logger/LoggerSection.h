// LoggerSection.h

#ifndef _FRAMEWORK_LOGGER_SECTION_H_
#define _FRAMEWORK_LOGGER_SECTION_H_

#include "framework/logger/Logger.h"

namespace framework
{
    namespace logger
    {

        class LoggerSection
        {
        public:
            LoggerSection(
                Logger & logger = global_logger())
                : logger_(logger)
                , started_(false)
            {
                start();
            }

            ~LoggerSection()
            {
                stop();
            }

            void start()
            {
                logger_.log_sec_beg();
                started_ = true;
            }

            void stop()
            {
                if (started_) {
                    logger_.log_sec_end();
                    started_ = false;
                }
            }

        private:
            Logger & logger_;
            bool started_;
        };

    } // namespace logger
} // namespace framework

#define LOG_SECTION() \
    framework::logger::LoggerSection _log_sec

#endif // _FRAMEWORK_LOGGER_SECTION_H_
