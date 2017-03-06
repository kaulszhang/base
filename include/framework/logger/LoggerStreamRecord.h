// LoggerStreamRecord.h

#ifndef _FRAMEWORK_LOGGER_STREAM_RECORD_H_
#define _FRAMEWORK_LOGGER_STREAM_RECORD_H_

#include "framework/logger/Logger.h"
#include "framework/logger/LoggerParam.h"

#include <sstream>

namespace framework
{
    namespace logger
    {

        template <
            typename _Front, 
            typename _Back>
        class LoggerStreamPair;

        template <
            typename _Ty
        >
        class LoggerStream;

        class LoggerStreamBeg
        {
        public:
            template <
                typename Back
            >
            LoggerStreamPair<LoggerStreamBeg, LoggerStream<Back> > const operator()(
                Back const & obj) const;

            template <
                typename Back
            >
            LoggerStreamPair<LoggerStreamBeg, LoggerStream<Back> > const operator<<(
                Back const & obj) const
            {
#ifdef __APPLE__
                return (*this).template operator()<Back>(obj);
#else
                return (*this).operator()<Back>(obj);
#endif
            }

        public:
            void format(
                std::ostream & os) const
            {
            }
        };

        template <
            typename _Ty
        >
        class LoggerStream
        {
        public:
            LoggerStream(
                _Ty const & value)
                : value_(value)
            {
            }

        public:
            void format(
                std::ostream & os) const
            {
                os << value_;
            }

        private:
            typename LoggerParam<_Ty>::store_type value_;
        };

        template <
            typename _Front, 
            typename _Back>
        class LoggerStreamPair
        {
        public:
            LoggerStreamPair(
                _Front const & front, 
                _Back const & back)
                : front_(front)
                , back_(back)
            {
            }

        public:
            typedef LoggerStreamPair this_type;
            typedef _Front front_type;
            typedef _Back back_type;

            template <
                typename Back
            >
            LoggerStreamPair<this_type, LoggerStream<Back> > const operator()(
                Back const & back) const
            {
                return LoggerStreamPair<this_type, LoggerStream<Back> >(*this, LoggerStream<Back>(back));
            }

            //template <
            //    size_t size
            //>
            //LoggerStreamPair<this_type, LoggerStream<char *> > const operator()(
            //    char const (& back)[size]) const
            //{
            //    return LoggerStreamPair<this_type, LoggerStream<_Back> >(*this, LoggerStream<char *>(back));
            //}

            template <
                typename Back
            >
            LoggerStreamPair<this_type, LoggerStream<Back> > const operator<<(
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
                std::ostream & os) const
            {
                front_.format(os);
                back_.format(os);
            }

        private:
            front_type front_;
            back_type back_;
        };

        template <
            typename Back
        >
        LoggerStreamPair<LoggerStreamBeg, LoggerStream<Back> > const LoggerStreamBeg::operator()(
            Back const & back) const
        {
            return LoggerStreamPair<LoggerStreamBeg, LoggerStream<Back> >(*this, LoggerStream<Back>(back));
        }

        //template <
        //    size_t size
        //>
        //LoggerStreamPair<LoggerStreamBeg, LoggerStream<char *> > const LoggerStreamBeg::operator()(
        //    char const (& back)[size]) const
        //{
        //    return LoggerStreamPair<LoggerStreamBeg, LoggerStream<_Back> >(*this, LoggerStream<char *>(back));
        //}

        template <
            typename _Pr
        > 
        class LoggerStreamRecord
            : private _Pr
            , public LoggerRecord
        {
        public:
            LoggerStreamRecord(
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
                LoggerStreamRecord const & me = 
                    static_cast<LoggerStreamRecord const &>(base);
                std::ostringstream oss;
                me.format(oss);
                strncpy(msg, oss.str().c_str(), len);
                return oss.str().size() > len ? len : oss.str().size();
            }

            static void destroy_self(
                LoggerRecord const & base)
            {
                LoggerStreamRecord const & me = 
                    static_cast<LoggerStreamRecord const &>(base);
                delete &me;
                (void)me;
            }
        };

        template <
            typename _Pr
        >
        inline LoggerStreamRecord<_Pr> const logger_stream_record(
            _Pr const & params)
        {
            return LoggerStreamRecord<_Pr>(params);
        }


    } // namespace logger
} // namespace framework

#define LOG_S(level, params) \
    framework::logger::Logger::log(_slogm(), level, framework::logger::logger_stream_record( \
        framework::logger::LoggerStreamBeg() << params))

#endif // _FRAMEWORK_LOGGER_STREAM_RECORD_H_
