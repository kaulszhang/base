// LoggerFormatRecord.h

#ifndef _FRAMEWORK_LOGGER_FORMAT_RECORD_H_
#define _FRAMEWORK_LOGGER_FORMAT_RECORD_H_

#include "framework/logger/Logger.h"
#include "framework/logger/LoggerParam.h"

#include <boost/format.hpp>

namespace framework
{
    namespace logger
    {

        template <
            typename _Front, 
            typename _Back
        >
        class LoggerFormats;

        template <
            typename _Ty
        >
        class LoggerFormat
        {
        public:
            LoggerFormat(
                _Ty const & value)
                : value_(value)
            {
            }

        public:
            void format(
                boost::format & fmt) const
            {
                fmt % value_;
            }

        private:
            typename LoggerParam<_Ty>::store_type value_;
        };

        class LoggerFormatsBeg
        {
        public:
            LoggerFormatsBeg(
                char const * fmt)
                : fmt_(fmt)
            {
            }

            template <
                typename Back
            >
            LoggerFormats<LoggerFormatsBeg, LoggerFormat<Back> > const operator()(
                Back const & nvp) const;

            template <
                typename Back
            >
            LoggerFormats<LoggerFormatsBeg, LoggerFormat<Back> > const operator%(
                Back const & nvp) const
            {
                return (*this).operator()<Back>(nvp);
            }

        public:
            void format(
                boost::format & fmt) const
            {
                fmt.parse(fmt_);
            }

        private:
            char const * fmt_;
        };

        class LoggerFormatsFmt
        {
        public:
            LoggerFormatsBeg const operator%(
                char const * fmt) const
            {
                return LoggerFormatsBeg(fmt);
            }
        };

        template <
            typename _Front, 
            typename _Back
        >
        class LoggerFormats
        {
        public:
            LoggerFormats(
                _Front const & front, 
                _Back const & back)
                : front_(front)
                , back_(back)
            {
            }

        public:
            typedef LoggerFormats this_type;
            typedef _Front front_type;
            typedef _Back back_type;

            template <
                typename Back
            >
            LoggerFormats<this_type, LoggerFormat<Back> > const operator()(
                Back const & back) const
            {
                return LoggerFormats<this_type, LoggerFormat<Back> >(*this, back);
            }

            template <
                typename Back
            >
            LoggerFormats<this_type, LoggerFormat<Back> > const operator%(
                Back const & back) const
            {
#ifdef __APPLE__
                return (*this).template operator()<Back>(back);
#else
                return (*this).operator()<Back>(back);
#endif
            }

        public:
            void format(
                boost::format & fmt) const
            {
                front_.format(fmt);
                back_.format(fmt);
            }

        private:
            front_type front_;
            back_type back_;
        };

        template <
            typename Back
        >
        LoggerFormats<LoggerFormatsBeg, LoggerFormat<Back> > const LoggerFormatsBeg::operator()(
            Back const & back) const
        {
            return LoggerFormats<LoggerFormatsBeg, LoggerFormat<Back> >(*this, back);
        }

        template <
            typename _Pr
        > 
        class LoggerFormatRecord
            : private _Pr
            , public LoggerRecord
        {
        public:
            LoggerFormatRecord(
                _Pr const & params)
                : _Pr(params)
                , LoggerRecord(format_message, destroy_self)
            {
            }

        private:
            static size_t format_message(
                LoggerRecord const & base, 
                char * msg, 
                size_t len)
            {
                LoggerFormatRecord const & me = 
                    static_cast<LoggerFormatRecord const &>(base);
                boost::format fmt;
                me.format(fmt);
                strncpy(msg, fmt.str().c_str(), len);
                return fmt.str().size() > len ? len : fmt.str().size();
            }

            static void destroy_self(
                LoggerRecord const & base)
            {
                LoggerFormatRecord const & me = 
                    static_cast<LoggerFormatRecord const &>(base);
                //delete &me;
                (void)me;
            }
        };

        template <
            typename _Pr
        >
        inline LoggerFormatRecord<_Pr> const logger_format_record(
            _Pr const & params)
        {
            return LoggerFormatRecord<_Pr>(params);
        }

    } // namespace logger
} // namespace framework

#define LOG_F(level, params) \
    framework::logger::Logger::log(_slogm(), level, framework::logger::logger_format_record( \
        framework::logger::LoggerFormatsFmt() % params))

#endif // _FRAMEWORK_LOGGER_FORMAT_RECORD_H_
