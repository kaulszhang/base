// LoggerListRecord.h

#ifndef _FRAMEWORK_LOGGER_LIST_RECORD_H_
#define _FRAMEWORK_LOGGER_LIST_RECORD_H_

#include "framework/logger/Logger.h"
#include "framework/logger/LoggerParam.h"
#include "framework/string/Format.h"
#include "framework/string/FormatStl.h"

#include <boost/preprocessor/seq/for_each.hpp>

namespace framework
{
    namespace logger
    {

        template <
            typename _Front, 
            typename _Back>
        class LoggerLists;

        class LoggerListsBeg
        {
        public:
            template <
                typename Back
            >
            LoggerLists<LoggerListsBeg, Back> const operator()(
                Back const & nvp) const;

            template <
                typename Back
            >
            LoggerLists<LoggerListsBeg, Back> const operator<<(
                Back const & nvp) const
            {
                return (*this).operator()<Back>(nvp);
            }

        public:
            void format(
                std::string & str) const
            {
                str += "(";
            }
        };

        template <
            typename _Ty
        >
        class LoggerList
        {
        public:
            LoggerList(
                char const * name, 
                _Ty const & value)
                : name_(name)
                , value_(value)
            {
            }

        public:
            void format(
                std::string & str) const
            {
                str += name_;
                str += "=";
                str += framework::string::format(value_);
            }

        private:
            char const * name_;
            typename LoggerParam<_Ty>::store_type value_;
        };

        class LoggerListsEnd
        {
        public:
            void format(
                std::string & str) const
            {
                str += ")";
            }
        };

        template <
            typename _Front, 
            typename _Back
        >
        class LoggerLists
            : _Front
            , _Back
        {
        public:
            LoggerLists(
                _Front const & front, 
                _Back const & back)
                : _Front(front)
                , _Back(back)
            {
            }

        public:
            typedef LoggerLists this_type;
            typedef _Front front_type;
            typedef _Back back_type;
            typedef _Back param_type;

            template <
                typename Back
            >
            LoggerLists<this_type, Back> const operator()(
                Back const & back) const
            {
                return LoggerLists<this_type, Back>(*this, back);
            }

            template <
                typename Back
            >
            LoggerLists<this_type, Back> const operator<<(
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
                std::string & str) const
            {
                front_type::format(str);
                str += ",";
                back_type::format(str);
            }
        };

        template <
            typename _Back
        >
        class LoggerLists<LoggerListsBeg, _Back>
            : LoggerListsBeg
            , _Back
        {
        public:
            LoggerLists(
                LoggerListsBeg const & beg, 
                _Back const & nvp)
                : LoggerListsBeg(beg)
                , _Back(nvp)
            {
            }

        public:
            typedef LoggerLists this_type;
            typedef LoggerListsBeg front_type;
            typedef _Back back_type;
            typedef _Back param_type;

            template <
                typename Back
            >
            LoggerLists<this_type, Back> const operator()(
                Back const & back) const
            {
                return LoggerLists<this_type, Back>(*this, back);
            }

            template <
                typename Back
            >
            LoggerLists<this_type, Back> const operator<<(
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
                std::string & str) const
            {
                front_type::format(str);
                back_type::format(str);
            }
        };

        template <
            typename _Front>
        class LoggerLists<_Front, LoggerListsEnd>
            : _Front
            , LoggerListsEnd
        {
        public:
            LoggerLists(
                _Front const & vec, 
                LoggerListsEnd const & end)
                : _Front(vec)
                , LoggerListsEnd(end)
            {
            }

        public:
            typedef LoggerLists this_type;
            typedef _Front front_type;
            typedef LoggerListsEnd back_type;

            void format(
                std::string & str) const
            {
                front_type::format(str);
                back_type::format(str);
            }
        };

        template <>
        class LoggerLists<LoggerListsBeg, LoggerListsEnd>
        {
        public:
            LoggerLists(
                LoggerListsBeg const & beg, 
                LoggerListsEnd const & end)
            {
            }

        public:
            typedef LoggerLists this_type;
            typedef LoggerListsBeg front_type;
            typedef LoggerListsEnd back_type;

            void format(
                std::string & str) const
            {
            }
        };

        template <
            typename Back
        >
        LoggerLists<LoggerListsBeg, Back> const LoggerListsBeg::operator()(
            Back const & back) const
        {
            return LoggerLists<LoggerListsBeg, Back>(*this, back);
        }

        template <
            typename _Pr
        > 
        class LoggerListRecord
            : private _Pr
            , public LoggerRecord
        {
        public:
            LoggerListRecord(
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
                LoggerListRecord const & me = 
                    static_cast<LoggerListRecord const &>(base);
                std::string str;
                me.format(str);
                strncpy(msg, str.c_str(), len);
                return str.size() > len ? len : str.size();
            }

            static void destroy_self(
                LoggerRecord const & base)
            {
                LoggerListRecord const & me = 
                    static_cast<LoggerListRecord const &>(base);
                //delete &me;
                (void)me;
            }
        };

        template <
            typename _Pr
        >
        inline LoggerListRecord<_Pr> const logger_list_record(
            _Pr const & params)
        {
            return LoggerListRecord<_Pr>(params);
        }

        template <
            typename _Ty
        >
        LoggerList<_Ty> const make_nvp(
            char const * name, 
            _Ty const & value)
        {
            return LoggerList<_Ty>(name, value);
        }


    } // namespace logger
} // namespace framework

#define LOG_STRINGLIZE(s) #s

#define LOG_NVP(v) \
    framework::logger::make_nvp(LOG_STRINGLIZE(v), v)

#define LOG_PARAM(z, d, p) << LOG_NVP(p)

#define LOG_L(level, params) \
    framework::logger::Logger::log(_slogm(), level, framework::logger::logger_list_record( \
        framework::logger::LoggerListsBeg() BOOST_PP_SEQ_FOR_EACH(LOG_PARAM, _, params) << framework::logger::LoggerListsEnd()))

#endif // _FRAMEWORK_LOGGER_LIST_RECORD_H_
