// Logger.h

/** 日志模块，一个支持多输出流和等级过滤的日志系统。

日志由日志管理对象（Logger的实例）进行处理。一个日志管理对象包含若干个日志输出流（LogStream）

有三种类型的输出流：
* 文件输出流：向普通文件输出
* 终端输出流：向标准输出输出
* UDP输出流：向某个UDP端口输出

输出流还定义了其他一些控制参数：
* Append：是否采用追加模式输出，只对文件输出流有效
* Color：是否带有颜色、下划线等控制转义字符
* Level：可以接受的最大日志信息等级

日志输出的记录格式如下：
日志记录项  = 时间戳 + CRLF + (日志信息项 + CRLF)(多个)
时间戳      = <YYYY-MM-DD hh:mm:ss>
日志信息项  = 中括号 + 消息级别 + 中括号 + 消息字符串
日志信息项  = 符号* + 结构化信息
日志信息项  = 符号# + 以十六进制显示的信息
消息级别    = ERROR|ALARM|EVENT|INFOR|DEBUG

关于消息级别，一般由如下约定：
* ERROR：  等级0，内部严重错误，必须重启，实际程序可能已经自动结束
* ALARM：  等级1，内部一般错误及外部（其他模块或者其他设备）错误，应检查网络连接状态和相关模块和设备的状况
* EVENT：  等级2，发生的事件记录，正常
* INFOR：  等级3，事件相关信息，较详细的信息，正常
* DEBUG：  等级4、5、6，调试信息

日志信息的等级由消息级别和一个等级微调值共同决定（两者之和），微调值可以调整信息项的等级，
以更加精确的过滤部分消息，微调值不改变消息级别本身。

在通过日志管理对象输出日志记录时，时间戳是由管理对象自动加上的，只需要输出每一个日志信息项，
但是需要告诉管理对象在一组日志信息项中哪一项是日志记录的开始项，哪一项是日志记录的结束项。输出
每个日志信息项时都需指定消息级别和等级微调值（默认为0），
*/

#ifndef _FRAMEWORK_LOGGER_LOGGER_H_
#define _FRAMEWORK_LOGGER_LOGGER_H_

#include "framework/logger/LoggerRecord.h"

#include <stdarg.h>

namespace framework
{
    namespace configure
    {
        class ConfigModule;
        class Config;
    }

    namespace logger
    {
#define LOGS_START  "["
#define LOGS_END    "]"

        // 将消息级别和等级微调值组合成一个数值进行传递
#define MAKE_LOG_LEVEL(level, sub_level) (level | (sub_level << 4))

        class IWriteStream;
        class Logger
        {
        public:
            enum
            {
                kLevelError		= 0,	// 消息级别的定义
                kLevelAlarm, 
                kLevelEvent, 
                kLevelInfor, 
                kLevelDebug, 
                kLevelDebug1, 
                kLevelDebug2, 
                kLevelNone, 
                kSubLevel0		= 0x00, // 等级微调值的定义
                kSubLevel1		= 0x10, 
                kSubLevel2		= 0x20, 
                kSubLevel3		= 0x30, 
                kLogStart		= 0x0100, // 日志记录开始的标志
                kLogEnd			= 0x0200, // 日志记录结束的标志
                kLogSingle = kLogStart | kLogEnd, // 单个日志信息项组成的日志记录，既是开始项，又是结束项
            };

        public:
            Logger();
            ~Logger();

        public:
            // 从配置文件读取输出流的定义
            boost::system::error_code load_config(
                framework::configure::Config & conf);

            void pause();

            void resume();

            void log_sec_beg() {++sec_ver_;}

            void log_sec_end() {++sec_ver_;}

            // 增加一个输出流
            bool add_stream(
                IWriteStream * os );

            // 删除一个输出流
            bool del_stream( IWriteStream * os );

        public:
            class LogModule
            {
                friend class Logger;

                LogModule(
                    Logger & logger)
                    : logger(logger)
                {
                }

                LogModule * next;
                Logger & logger;
                char const * name;
                size_t level;
            };

            LogModule & register_module(
                char const * name, 
                size_t level = 2); // the default level

        public:
            template <typename _Rec>
            static inline void log(
                LogModule const & module, 
                size_t level, 
                _Rec const & record)
            {
                if ( !( &module ) ) return;/// 防止在多线程环境下，一个模块还未注册，另一个线程访问出错
                if (level + module.level > module.logger.max_level_ || level >= kLevelNone) {
                    return;
                }
                module.logger.private_log(module, level, record);
            }

            // 输出一条采用结构化信息格式的日志信息
            static inline void print_struct(
                LogModule const & module, 
                size_t level, 
                char const * title, 
                char const * fmt, 
                void const * var)
            {
                if ( !( &module ) ) return;
                module.logger.private_print_struct(module, level, title, fmt, var);
            }

            // 输出一条内容较长的日志信息，如果消息内容可能超过1024字节，使用此接口，否则会被中间截断
            static inline void print_string(
                LogModule const & module, 
                size_t level, 
                char const * txt)
            {
                if ( !( &module ) ) return;
                module.logger.private_print_string(module, level, txt);
            }

            static inline void print_string(
                LogModule const & module, 
                size_t level, 
                char const * txt, 
                size_t len)
            {
                if ( !( &module ) ) return;
                module.logger.private_print_string(module, level, txt, len);
            }

            // 输出一条采用16进制信息格式的日志信息
            static inline void print_hex(
                LogModule const & module, 
                size_t level, 
                unsigned char const * data, 
                size_t len)
            {
                if ( !( &module ) ) return;
                module.logger.private_print_hex(module, level, data, len);
            }

        private:
            struct LogStream;

            struct LogDefine;

            // 增加一个输出流
            bool add_stream(
                LogStream * ls);

            bool backup_open_file(
                LogStream * ls);

        private:
            void at_new_time();

            void private_log(
                LogModule const & module, 
                size_t level, 
                LoggerRecord const & record);

            // 输出一条采用结构化信息格式的日志信息
            void private_print_struct(
                LogModule const & module, 
                size_t level, 
                char const * title, 
                char const * fmt, 
                void const * var);

            // 输出一条内容较长的日志信息，如果消息内容可能超过1024字节，使用此接口，否则会被中间截断
            void private_print_string(
                LogModule const & module, 
                size_t level, 
                char const * txt);

            void private_print_string(
                LogModule const & module, 
                size_t level, 
                char const * txt, 
                size_t len);

            // 输出一条采用16进制信息格式的日志信息
            void private_print_hex(
                LogModule const & module, 
                size_t level, 
                unsigned char const * data, 
                size_t len);

        private:
            /// 线程锁
            class Lock;
            Lock * lock_;

            /// 今天开始时刻
            time_t mid_night_;

        private:
            size_t sec_ver_;
            size_t time_sec_ver_;
            char time_str_[40];
            char msg_str_[1024];
            size_t max_level_;
            configure::ConfigModule * conf_;
            LogStream * stream_lst_;
            LogStream * stream_lst_bak_;
            LogModule * module_lst_;
            LogDefine * log_def_;
            bool paused_;
        };

        extern Logger & glog;

        Logger & global_logger();

        inline static Logger & _slog(...)
        {
            return global_logger();
        }

    } // namespace logger
} // namespace framework

#define FRAMEWORK_LOGGER_DECLARE_LOGGER(name) \
    inline static framework::logger::Logger & _slog() \
{ \
    static framework::logger::Logger log; \
    return log; \
}

#define FRAMEWORK_LOGGER_DECLARE_MODULE(name) \
    inline static framework::logger::Logger::LogModule const & _slogm() \
{ \
    using framework::logger::_slog; \
    static framework::logger::Logger::LogModule const & module = \
    _slog().register_module(name); \
    return module; \
}

#define FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL(name, lvl) \
    inline static framework::logger::Logger::LogModule const & _slogm() \
{ \
    using framework::logger::_slog; \
    static framework::logger::Logger::LogModule const & module = \
    _slog().register_module(name, lvl); \
    return module; \
}

#define FRAMEWORK_LOGGER_DECLARE_MODULE_USE_BASE(base) \
    using base::_slogm

#define LOG_PKT(lvl, name, fmt, dat) \
    framework::logger::Logger::print_struct(_slogm(), lvl, name, fmt, dat)

#define LOG_STR(lvl, str) \
    framework::logger::Logger::print_string(_slogm(), lvl, str)

#define LOG_STR_LEN(lvl, str, len) \
    framework::logger::Logger::print_string(_slogm()(, lvl, str, len)

#define LOG_HEX(lvl, dat, len) \
    framework::logger::Logger::print_hex(_slogm(), lvl, dat, len)

#endif // _FRAMEWORK_LOGGER_LOGGER_H_
