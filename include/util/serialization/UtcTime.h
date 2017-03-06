// UtcTime.h

#ifndef _UTIL_SERIALIZATION_UTCTIME_H_
#define _UTIL_SERIALIZATION_UTCTIME_H_

#include <framework/string/Slice.h>
#include <framework/timer/TimeConvert.h>

namespace util
{
    namespace serialization
    {
        struct UtcTime
        {
        public:
            UtcTime()
                : time_(0)
            {
            }

        public:
            boost::system::error_code from_string(
                std::string const & str)
            {
                time_ = UTCtoLocaltime(str);
                return boost::system::error_code();
            }

        public:
            time_t to_time_t() const
            {
                return time_;
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                std::string value;
                ar & value;
                from_string(value);
            }

        private:
            static time_t UTCtoLocaltime(
                std::string const & utctime)
            {
                static std::map<std::string, int> week1;
                static std::map<std::string, int> months1;
                if (week1.empty()) {
                    week1["Sun"] = 0;
                    week1["Mon"] = 1;
                    week1["Tue"] = 2;
                    week1["Wed"] = 3;
                    week1["Thu"] = 4;
                    week1["Fri"] = 5;
                    week1["Sat"] = 6;

                    months1["Jan"] = 0;
                    months1["Feb"] = 1;
                    months1["Mar"] = 2;
                    months1["Apr"] = 3;
                    months1["May"] = 4;
                    months1["Jun"] = 5;
                    months1["Jul"] = 6;
                    months1["Aug"] = 7;
                    months1["Sep"] = 8;
                    months1["Oct"] = 9;
                    months1["Nov"] = 10;
                    months1["Dec"] = 11;
                }

                std::vector<std::string> Res;
                framework::string::slice<std::string>(utctime, std::inserter(Res, Res.end()), " ");
                std::vector<int> Res1;
                framework::string::slice<int>(Res[3], std::inserter(Res1, Res1.end()), ":");
                tm m_tm;
                if(Res[4] == "CST")
                    m_tm.tm_year   = atoi(Res[5].c_str())-1900;
                else
                    m_tm.tm_year   = atoi(Res[4].c_str())-1900;
                m_tm.tm_mon    = months1[Res[1]];
                m_tm.tm_yday   = 0;
                m_tm.tm_mday   = atoi(Res[2].c_str());
                m_tm.tm_wday   = week1[Res[0]];
                m_tm.tm_hour   = Res1[0];
                m_tm.tm_min    = Res1[1];
                m_tm.tm_sec    = Res1[2];
                m_tm.tm_isdst  = -1;
                return framework::timer::time_gm(&m_tm);
            }

        private:
            time_t time_;
        };

    }
}

#endif // _UTIL_SERIALIZATION_UTCTIME_H_
