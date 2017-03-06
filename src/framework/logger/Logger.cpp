// Logger.cpp

#include "framework/Framework.h"
#include "framework/logger/Logger.h"
#include "framework/configure/Config.h"
#include "framework/thread/NullLock.h"
#include "framework/logger/LogMsgStream.h"
#include "framework/network/Connector.h"
#include "framework/filesystem/Path.h"
using namespace framework::configure;
using namespace framework::system::logic_error;

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/thread/tss.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio/ip/udp.hpp>
using namespace boost::system;

#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef BOOST_WINDOWS_API
#  if (defined UNDER_CE) || (defined __MINGW32__)
#    define localtime_r(x, y) *y = *localtime(x)
#  else 
#    define localtime_r(x, y) localtime_s(y, x)
#  endif
#  define snprintf _snprintf
#  define mkdir(x, y) mkdir(x)
#endif

namespace framework
{
    namespace logger
    {

#define DATE_START  1
#define TIME_START  (DATE_START + 11)

        char const * const level_str[] = {"ERROR", "ALARM", "EVENT", "INFOR", "DEBUG", "DEBUG", "DEBUG"};
        char const clr[8] = {'1', '3', '2', '4', '6', '6', '6'};
        char clr_fmt_msg[24] = "\033[31m[%s]\033[0m [%s] %s\n";

#define LOG_LEVEL(x) ((x & 0x000f) + ((x & 0x00f0) >> 4))
#define LOG_REAL_LEVEL(x) (x & 0x000f)

        Logger & global_logger()
        {
            static Logger slog;
            return slog;
        }

        Logger & glog = global_logger();

        /// 获取当前时间串
        ///
        ///     获取当前的时间，转换为指定的字符串格式
        ///     @param buf 用以接收格式化后时间的字符串
        ///     @param size 缓冲大小
        ///     @param fmt 时间格式
        ///     @return 格式化后的时间串
        static char const *time_str_now(
            char * buf, 
            size_t size, 
            char const * fmt)
        {
            time_t tt = time(NULL);
            struct tm lt;
            localtime_r(&tt, &lt);
            strftime(buf, size, fmt, &lt);
            return buf;
        }

        class Logger::Lock
            : public boost::mutex
        {
        };

        struct Logger::LogStream
        {
            LogStream * next;
            std::string log_file;
            size_t log_lvl;
            bool log_clr;
            bool log_app;
            bool log_day;
            bool log_roll;
            size_t log_size;
            size_t sec_ver;
            bool log_time;
            bool external;
            IWriteStream * log_fp;
        };

        struct Logger::LogDefine
        {
            bool log_pid;
            bool log_tid;
            boost::thread_specific_ptr<char> str;
        };

        Logger::Logger()
            : sec_ver_(0)
            , time_sec_ver_(size_t(-1))
            , max_level_(0)
            , conf_(NULL)
            , stream_lst_(NULL)
            , stream_lst_bak_(NULL)
            , module_lst_(NULL)
            , log_def_(new LogDefine)
            , paused_(false)
        {
            strncpy(time_str_, "<0000-00-00 00:00:00>", sizeof(time_str_));
            time_str_now(time_str_ + DATE_START, 11, "%Y-%m-%d");
            time_str_[DATE_START + 10] = ' ';
            time_t tt = time(NULL);
            struct tm lt;
            localtime_r(&tt, &lt);
            lt.tm_hour = lt.tm_min = lt.tm_sec = 0;
            mid_night_ = mktime(&lt);
            msg_str_[sizeof(msg_str_) - 1] = '\0';
            lock_ = new Lock;
        }

        Logger::~Logger()
        {
            if (paused_)
                stream_lst_ = stream_lst_bak_;
            while (LogStream * p = stream_lst_) {
                stream_lst_ = stream_lst_->next;
                if (p->log_fp && !p->external)
                    delete p->log_fp;
                delete p;
            }
            while (LogModule * p = module_lst_) {
                module_lst_ = module_lst_->next;
                delete p;
            }
            delete lock_;
            lock_ = NULL;
            delete log_def_;
            log_def_ = NULL;
        }

        error_code Logger::load_config(
            Config & conf)
        {
            int numLogStream = 0;
            ConfigModule & logger_config = conf.register_module("Logger");
            conf_ = &logger_config;
            logger_config << CONFIG_PARAM_NOACC(numLogStream);
            logger_config << CONFIG_PARAM_NAME_NOACC("stream_count", numLogStream);

            log_def_->log_pid = false;
            log_def_->log_tid = false;

            logger_config << CONFIG_PARAM_NAME_RDWR("max_level", max_level_);
            logger_config << CONFIG_PARAM_NAME_RDWR("log_pid", log_def_->log_pid);
            logger_config << CONFIG_PARAM_NAME_RDWR("log_tid", log_def_->log_tid);

            for (LogModule * p = module_lst_; p; p = p->next) {
                logger_config << CONFIG_PARAM_NAME_RDWR(p->name, p->level);
            }

            for (int i = 0; i < numLogStream; i++) {
                std::string LogStreamName = std::string("LogStream") + char(i + 0x30);

                LogStream * ls = new LogStream;

                ls->log_lvl = Logger::kLevelDebug2;
                ls->log_clr = false;
                ls->log_app = false;
                ls->log_day = false;
                ls->log_roll = false;
                ls->log_size = 0;
                ls->sec_ver = time_sec_ver_;
                ls->log_fp = NULL;
                ls->log_time = false;
                ls->external = false;
                ls->next = NULL;

                conf.register_module(LogStreamName)
                    << CONFIG_PARAM_NAME_RDONLY("logFile", ls->log_file)
                    << CONFIG_PARAM_NAME_RDONLY("file", ls->log_file)
                    << CONFIG_PARAM_NAME_RDONLY("logAppend", ls->log_app)
                    << CONFIG_PARAM_NAME_RDONLY("append", ls->log_app)
                    << CONFIG_PARAM_NAME_RDWR("logLevel", ls->log_lvl)
                    << CONFIG_PARAM_NAME_RDWR("level", ls->log_lvl)
                    << CONFIG_PARAM_NAME_RDWR("logColor", ls->log_clr)
                    << CONFIG_PARAM_NAME_RDWR("color", ls->log_clr)
                    << CONFIG_PARAM_NAME_RDWR("logDaily", ls->log_day)
                    << CONFIG_PARAM_NAME_RDWR("daily", ls->log_day)
                    << CONFIG_PARAM_NAME_RDWR("logRoll", ls->log_roll)
                    << CONFIG_PARAM_NAME_RDWR("roll", ls->log_roll)
                    << CONFIG_PARAM_NAME_RDWR("logSize", ls->log_size)
                    << CONFIG_PARAM_NAME_RDWR("size", ls->log_size);

                add_stream(ls);
            }

            stream_lst_bak_ = stream_lst_;
            if (paused_) {
                stream_lst_ = NULL;
            }
            return succeed;
        }

        // 增加一个输出流
        bool Logger::add_stream(
            IWriteStream * os )
        {
            if ( !os ) return false;

            Lock::scoped_lock streamLock(*lock_);
            LogStream * ls = new LogStream;

            (void)ls->log_file;
            ls->log_lvl = max_level_;
            ls->log_clr = false;
            ls->log_app = false;
            ls->log_day = false;
            ls->log_roll = false;
            ls->log_size = 0;
            ls->sec_ver = time_sec_ver_;
            ls->log_fp = os;
            ls->log_time = false;
            ls->external = true;
            ls->next = NULL;

            if ( !add_stream( ls ) ) return false;

            stream_lst_bak_ = stream_lst_;
            if (paused_) stream_lst_ = NULL;

            return true;
        }

        // 删除一个输出流
        bool Logger::del_stream( IWriteStream * os )
        {
            Lock::scoped_lock delstreamLock(*lock_);

            if ( !stream_lst_ ) return true;
            LogStream **pre = &stream_lst_;//, *next = stream_lst_;

            while ( *pre )
            {
                if ( (*pre)->log_fp == os ) {
                    LogStream *p = *pre;
                    *pre = p->next;
                    assert(p->external);
                    delete p;
                    break;
                }
                pre = &(*pre)->next;
            }
            return true;
        }

        bool Logger::add_stream(
            LogStream * ls)
        {
            if (ls->log_fp) {
                // do nothing
            } else if (!ls->log_file.empty()) {
                if (0) {
#if __linux__
                } else if (ls->log_file[0] == '#') {
                    //ls->log_fp = fdopen(atoi(ls->log_file.substr(1).c_str()), (ls->log_app) ? "a+" : "w+");
                    ls->log_fp->open( ls->log_file.c_str(), (ls->log_app) ? "a+" : "w+" );
                    ls->log_day = false;
                    ls->log_roll = false;
                    ls->log_size = 0;
                } else if (ls->log_file.substr(0, 9) == "/dev/udp/") {
                    std::string host = ls->log_file.substr(9);
                    std::string::size_type p = host.find('/');
                    int fd = -1;
                    if (p != std::string::npos) {
                        boost::asio::io_service io_svc;
                        boost::asio::detail::mutex mutex;
                        boost::asio::ip::udp::socket socket(io_svc);
                        framework::network::Connector<framework::network::InternetProtocol<boost::asio::ip::udp,boost::asio::ip::udp::socket>,boost::asio::ip::udp::socket > connector(io_svc, mutex);
                        framework::network::NetName addr;
                        error_code ec;
                        addr.host(host.substr(0, p).c_str());
                        addr.svc(host.substr(p + 1).c_str());
                        connector.connect(socket, addr, ec);
                        if (!ec)
                            fd = socket.native();
                    }
                    if (fd != -1)
                    {
                        char strfd[128] = { 0 };
                        snprintf( strfd, 128, "#%d", fd );
                        ls->log_fp->open( strfd, "w");
                        //ls->log_fp = fdopen(fd, "w");
                    }
                    ls->log_day = false;
                    ls->log_roll = false;
                    ls->log_size = 0;
#elif (defined BOOST_WINDOWS_API)
                } else if (ls->log_file == "DBGVIEW") {
                    ls->log_fp = new DebugStringStream;
                    ls->log_day = false;
                    ls->log_roll = false;
                    ls->log_size = 0;
#endif
                }
#if   defined(__ANDROID__)
                else if (ls->log_file == "LOGDOUT") {
                    ls->log_fp = new OLOGDStream;
                    ls->log_day = false;
                    ls->log_roll = false;
                    ls->log_size = 0;
                }
#endif
                else if (ls->log_file == "STDOUT") {
                    ls->log_fp = new OStdStream;
                    ls->log_day = false;
                    ls->log_roll = false;
                    ls->log_size = 0;
                } else {
                    backup_open_file(ls);
                }
            } else {// 输出文件名为空，输出到标准输出
                ls->log_fp = new OStdStream;
            }

            if (ls->log_fp == NULL) {
                delete ls;
                return false;
            }
            LogStream ** p = &stream_lst_;
            while (*p)
                p = &(*p)->next;
            *p = ls;
            if (ls->log_lvl > max_level_)
                max_level_ = (int)ls->log_lvl;
            return true;
        }

        bool Logger::backup_open_file(
            LogStream * ls)
        {
            bool log_app = ls->log_app;
            if (ls->log_fp) {
                delete ls->log_fp;
                //fclose(ls->log_fp);
                log_app = false;
            }
            if (ls->log_file[0] == '$' 
                && ls->log_file[1] == 'L' 
                && ls->log_file[2] == 'O' 
                && ls->log_file[3] == 'G') {
                    ls->log_file = 
                        framework::filesystem::log_path().string() 
                        + ls->log_file.substr(4);
            }
            if (!log_app) {
                std::string::size_type p = ls->log_file.rfind('/');
                char buf[40];
                time_str_now(buf, sizeof(buf), ".%Y-%m-%d %HH%MM%SS");

                try
                {
                    if (p == std::string::npos) {
                        boost::filesystem::create_directory("log_bak");
                        if (boost::filesystem::exists(ls->log_file))
                            boost::filesystem::rename(ls->log_file, "log_bak/" + ls->log_file + buf);
                    } else {
                        boost::filesystem::create_directories((ls->log_file.substr(0, p) + "/log_bak").c_str());
                        if (boost::filesystem::exists(ls->log_file))
                            boost::filesystem::rename(ls->log_file, ls->log_file.substr(0, p) + "/log_bak" + ls->log_file.substr(p) + buf);
                    }
                }
                catch (...)
                {
                }
            }
            // 不能用append模式打开，不然无法回滚
            ls->log_fp = new OFileStream;
            int opflag = ls->log_fp->open(ls->log_file.c_str(), "r+");
            if (opflag == 0)
                opflag = ls->log_fp->open(ls->log_file.c_str(), "w+");
            if (opflag && log_app)
                ls->log_fp->seek(0, SEEK_END);
            return true;
        }

        void Logger::pause()
        {
            paused_ = true;
            stream_lst_ = NULL;
        }

        void Logger::resume()
        {
            paused_ = false;
            stream_lst_ = stream_lst_bak_;
        }

        Logger::LogModule & Logger::register_module(
            char const * name, 
            size_t level)
        {
            LogModule * m = NULL;
            Lock::scoped_lock locker(*lock_);
            for (LogModule * p = module_lst_; p; p = p->next) {
                if (strcmp(name, p->name) == 0) {
                    m = p;
                    break;
                }
            }
            if (m == NULL) {
                m = new LogModule(*this);
                m->name = name;
                m->level = level;
                m->next = module_lst_;
                module_lst_ = m;
                if (conf_) {
                    (*conf_) << CONFIG_PARAM_NAME_RDWR(m->name, m->level);
                }
            }
            return *m;
        }

        void Logger::at_new_time()
        {
            int t_diff = (int)(time(NULL) - mid_night_);
            if (t_diff >= 24 * 60 * 60) {
                // 双保险
                Lock::scoped_lock locker(*lock_);
                t_diff = (int)(time(NULL) - mid_night_);
                if (t_diff >= 24 * 60 * 60) {
                    for (LogStream * p = stream_lst_; p; p = p->next) {
                        if (p->log_day) {
                            backup_open_file(p);
                        }
                        if (p->log_size && (size_t)p->log_fp->tell() > p->log_size) {
                            backup_open_file(p);
                        }
                    }
                    time_str_now(time_str_ + DATE_START, 11, "%Y-%m-%d");
                    time_str_[DATE_START + 10] = ' ';
                    mid_night_ += 24 * 60 * 60;
                    t_diff -= 24 * 60 * 60;
                }
            }

            for (LogStream * p = stream_lst_; p; p = p->next) {
                p->log_fp->flush();
                if (!p->log_file.empty() && p->log_size && (size_t)p->log_fp->tell() > p->log_size) {
                    if (p->log_roll) {
                        p->log_fp->seek(0, SEEK_SET);
                    } else {
                        backup_open_file(p);
                    }
                }
            }

            int hour = t_diff / 3600;
            int minute = t_diff % 3600;
            int second = minute % 60;
            minute /= 60;
            time_str_[TIME_START] = hour / 10 + '0';
            time_str_[TIME_START + 1] = hour % 10 + '0';
            time_str_[TIME_START + 3] = minute / 10 + '0';
            time_str_[TIME_START + 4] = minute % 10 + '0';
            time_str_[TIME_START + 6] = second / 10 + '0';
            time_str_[TIME_START + 7] = second % 10 + '0';
            time_sec_ver_ = sec_ver_;
        }

        void Logger::private_log(
            LogModule const & module, 
            size_t level, 
            LoggerRecord const & record)
        {
            size_t sub_level = module.level;

            /// 栈上申请，防止信息覆盖。
            char msg_str_[1024] = { 0 };
            if (level + sub_level <= max_level_ && level < kLevelNone) {
                record.format_message(msg_str_, sizeof(msg_str_) - 1);
                clr_fmt_msg[3] = clr[LOG_REAL_LEVEL(level)];
            } else {
                return;
            }

            for (LogStream * p = stream_lst_; p; p = p->next) {
                if (level + sub_level <= p->log_lvl) {
                    if (p->sec_ver != sec_ver_) {
                        if (time_sec_ver_ != sec_ver_)
                            at_new_time();
                        if (p->log_clr) {
                            char szbuf[1024] = { 0 };
                            sprintf(szbuf, "\033[4;37m%s\033[0m\n", time_str_);
                            p->log_fp->write( szbuf );
                        } else {
                            char szbuf[1024] = { 0 };
                            sprintf(szbuf, "%s\n", time_str_);
                            p->log_fp->write( szbuf );
                        }
                        p->sec_ver = sec_ver_;
                    }
                    if (p->log_clr) {
                        char szbuf[1024] = { 0 };
                        snprintf(szbuf, sizeof(szbuf), clr_fmt_msg, level_str[level], module.name, msg_str_);
                        szbuf[sizeof(szbuf) - 1] = '\0';
                        szbuf[sizeof(szbuf) - 2] = '\n';
                        p->log_fp->write( szbuf );
                    } else {
                        char szbuf[1024] = { 0 };
                        snprintf(szbuf, sizeof(szbuf), "[%s] [%s] %s\n", level_str[level], module.name, msg_str_);
                        szbuf[sizeof(szbuf) - 1] = '\0';
                        szbuf[sizeof(szbuf) - 2] = '\n';
                        p->log_fp->write( szbuf );
                    }
                    //fflush(p->log_fp);
                }
            }
        }

        static char const chex[] = "0123456789ABCDEF";

        void Logger::private_print_struct(
            LogModule const & module, 
            size_t level, 
            char const * title, 
            char const * fmt, 
            void const * var)
        {
            char const *pv, *pf, *pc;
            char *pm, *pmsg;
            char c, d = 0;
            int count, i;
            size_t lm = 0, l = 0;

            level += module.level;

            if (level> max_level_)
                return;

            if (title) {
                for (LogStream * p = stream_lst_; p; p = p->next)
                    if (level <= p->log_lvl)
                    {
                        char szbuf[1024] = { 0 };
                        sprintf(szbuf, "+ %s\n", title);
                        p->log_fp->write( szbuf );
                    }
            }
            if (fmt == NULL || var == NULL) {
                return;
            }
            pf = fmt;
            pv = (char*)var;
            while (*pf) {
                pc = strchr(pf, '%');
                if (pc == NULL || pc[1] == 0) {
                    break;
                }
                memset(msg_str_, ' ', 16);
                if (title) {
                    memcpy(msg_str_ + 2, pf, pc - pf);
                }
                else {
                    memcpy(msg_str_, pf, pc - pf);
                }
                pc++;
                count = 0;
                while (*pc >= '0' && *pc <= '9') {
                    count = count * 10 + (*pc - '0');
                    pc++;
                }
                if (count == 0) {
                    count = 1;
                } else {
                    if (*pc == '#') {
                        pc++;
                        d = *pc++;
                    } else {
                        d = 0;
                    }
                }
                c = *pc++;
                pm = msg_str_ + 16;
                lm = sizeof(msg_str_) - 16;
                for (i = 0; i < count; i++) {
                    switch (c) {
        case 'c':           /* char */
            *pm++ = *(char*)pv++;
            --lm;
            break;
        case 'b':           /* byte */
            *pm++ = chex[(*(unsigned char*)pv) >> 4];
            --lm;
            *pm++ = chex[(*(unsigned char*)pv++) & 0x0F];
            --lm;
            break;
        case 'w':           /* short */
            l = snprintf(pm, lm, "%d", *(short*)pv);
            pm += l;
            lm -= l;
            pv += 2;
            break;
        case 'W':           /* short */
            l = snprintf(pm, lm, "%d", ntohs(*(short*)pv));
            pm += l;
            lm -= l;
            pv += 2;
            break;
        case 'i':           /* id */
            memcpy(pm, pv, 16);
            pm += 16;
            lm -= 16;
            msg_str_[32] = 0;
            pv += 16;
            break;
            //case 'a':           /* ip */
            //	ip_str(*(IpAddr*)pv, pm, 40);
            //	pm += strlen(pm);
            //	pv += sizeof(IpAddr);
            //	break;
        case 'n':           /* number */
            l = snprintf(pm, lm, "%d", *(int*)pv);
            pm += l;
            lm -= l;
            pv += 4;
            break;
        case 'N':           /* number */
            l = snprintf(pm, lm, "%d", ntohl(*(int*)pv));
            pm += l;
            lm -= l;
            pv += 4;
            break;
        case 'h':           /* hex */
            l = snprintf(pm, lm, "0x%.8X", *(unsigned int*)pv);
            pm += l;
            lm -= l;
            pv += 4;
            break;
        case 'H':           /* hex */
            l = snprintf(pm, lm, "0x%.8X", ntohl(*(unsigned int*)pv));
            pm += l;
            lm -= l;
            pv += 4;
            break;
        case 's':           /* string */
            l = snprintf(pm, lm, "%s", pv);
            pmsg = pm;
            while ((pmsg = strchr(pmsg, '\n'))) {
                *pmsg++ = '\\';
            }
            pm += l;
            lm -= l;
            break;
                    }
                    if (d) {
                        *pm++ = d;
                        --lm;
                    }
                }
                if (d) {
                    pm[-1] = 0;
                }
                *pm = 0;
                if (*pf != '.') {
                    for (LogStream * p = stream_lst_; p; p = p->next) {
                        if (level <= p->log_lvl) {
                            char szbuf[1024] = { 0 };
                            sprintf(szbuf, "+ %s\n", msg_str_);
                            p->log_fp->write( szbuf );
                        }
                    }
                }
                pf = pc;
            }
            for (LogStream * p = stream_lst_; p; p = p->next) {
                if (level <= p->log_lvl) {
                    p->log_fp->flush();
                }
            }
        }

        void Logger::private_print_string(
            LogModule const & module, 
            size_t level, 
            char const * txt)
        {
            level += module.level;

            if (level > max_level_)
                return;

            for (LogStream * p = stream_lst_; p; p = p->next) {
                if (level <= p->log_lvl) {
                    char szbuf[1024] = { 0 };
                    snprintf(szbuf, sizeof(szbuf), "%s\n", txt);
                    szbuf[sizeof(szbuf) - 1] = '\0';
                    szbuf[sizeof(szbuf) - 2] = '\n';
                    p->log_fp->write( szbuf );
                }
            }
        }

        void Logger::private_print_string(
            LogModule const & module, 
            size_t level, 
            char const * txt, 
            size_t len)
        {
            level += module.level;

            if (level > max_level_)
                return;

            char fmt[32];
            snprintf(fmt, sizeof(fmt), "%%%ds\n", len);
            for (LogStream * p = stream_lst_; p; p = p->next) {
                if (level <= p->log_lvl) {
                    char szbuf[1024] = { 0 };
                    snprintf(szbuf, sizeof(szbuf), fmt, txt);
                    szbuf[sizeof(szbuf) - 1] = '\0';
                    szbuf[sizeof(szbuf) - 2] = '\n';
                    p->log_fp->write( szbuf );
                }
            }
        }

        void Logger::private_print_hex(
            LogModule const & module, 
            size_t level, 
            unsigned char const * data, 
            size_t len)
        {
            const size_t line_num_len = 4;
            const size_t line_num_start = 1;
            const size_t hex_start = line_num_len + 2;
            //const size_t text_start = hex_start + 52 + 1;

            level += module.level;

            if (level > max_level_)
                return;

            size_t line, byte, line_pos, data_pos;

            // 填入不变的部分
            line_pos = 0;
            msg_str_[line_pos++] = '[';
            line_pos += line_num_len;
            msg_str_[line_pos++] = ']';
            for (byte = 0; byte < 16; byte++) {
                if ((byte & 0x03) == 0) {
                    msg_str_[line_pos++] = ' ';
                }
                line_pos += 2;
                msg_str_[line_pos++] = ' ';
            }
            msg_str_[line_pos++] = '|';
            line_pos += 16;
            msg_str_[line_pos++] = '|';
            msg_str_[line_pos] = 0;

            // 开始工作
            data_pos = 0;
            for (line = 0; line < len / 16; line++) {
                size_t n = line;
                for (size_t i = line_num_len - 1; i != (size_t)-1; i--) {
                    msg_str_[line_num_start + i] = chex[n & 0x0F];
                    n >>= 4;
                }
                line_pos = hex_start;
                for (byte = 0; byte < 16; byte++, data_pos++, line_pos++) {
                    if ((byte & 0x03) == 0) {
                        line_pos++;
                    }
                    msg_str_[line_pos++] = chex[data[data_pos] >> 4];
                    msg_str_[line_pos++] = chex[data[data_pos] & 0x0F];
                }
                line_pos++; // skip border '|'
                for (data_pos -= 16, byte = 0; byte < 16; byte++, data_pos++, line_pos++) {
                    if (isgraph(data[data_pos]))
                        msg_str_[line_pos] = data[data_pos];
                    else
                        msg_str_[line_pos] = '.';
                }
                for (LogStream * p = stream_lst_; p; p = p->next)
                    if (level <= p->log_lvl)
                    {
                        char szbuf[1024] = { 0 };
                        sprintf(szbuf, "# %s\n", msg_str_);
                        p->log_fp->write( szbuf );
                    }
            }
            size_t n = line;
            for (size_t i = line_num_len - 1; i != (size_t)-1; i--) {
                msg_str_[line_num_start + i] = chex[n & 0x0F];
                n >>= 4;
            }
            for (line_pos = hex_start, byte = 0; byte < len % 16; byte++, data_pos++, line_pos++) {
                if ((byte & 0x03) == 0) {
                    line_pos++;
                }
                msg_str_[line_pos++] = chex[data[data_pos] >> 4];
                msg_str_[line_pos++] = chex[data[data_pos] & 0x0F];
            }
            for (data_pos -= len % 16; byte < 16; byte++, line_pos++) {
                if ((byte & 0x03) == 0) {
                    line_pos++;
                }
                msg_str_[line_pos++] = ' ';
                msg_str_[line_pos++] = ' ';
            }
            line_pos++; // skip border '|'
            for (byte = 0; byte < len % 16; byte++, data_pos++, line_pos++) {
                if (isgraph(data[data_pos]))
                    msg_str_[line_pos] = data[data_pos];
                else
                    msg_str_[line_pos] = '.';
            }
            for (; byte < 16; byte++, line_pos++) {
                msg_str_[line_pos] = ' ';
            }
            for (LogStream * p = stream_lst_; p; p = p->next) {
                if (level <= p->log_lvl) {
                    char szbuf[1024] = { 0 };
                    sprintf(szbuf, "# %s\n", msg_str_);
                    p->log_fp->write( szbuf );
                }
            }
        }

    } // namespace logger
} // namespace framework
