// Logger.h

/** ��־ģ�飬һ��֧�ֶ�������͵ȼ����˵���־ϵͳ��

��־����־�������Logger��ʵ�������д���һ����־�������������ɸ���־�������LogStream��

���������͵��������
* �ļ������������ͨ�ļ����
* �ն�����������׼������
* UDP���������ĳ��UDP�˿����

�����������������һЩ���Ʋ�����
* Append���Ƿ����׷��ģʽ�����ֻ���ļ��������Ч
* Color���Ƿ������ɫ���»��ߵȿ���ת���ַ�
* Level�����Խ��ܵ������־��Ϣ�ȼ�

��־����ļ�¼��ʽ���£�
��־��¼��  = ʱ��� + CRLF + (��־��Ϣ�� + CRLF)(���)
ʱ���      = <YYYY-MM-DD hh:mm:ss>
��־��Ϣ��  = ������ + ��Ϣ���� + ������ + ��Ϣ�ַ���
��־��Ϣ��  = ����* + �ṹ����Ϣ
��־��Ϣ��  = ����# + ��ʮ��������ʾ����Ϣ
��Ϣ����    = ERROR|ALARM|EVENT|INFOR|DEBUG

������Ϣ����һ��������Լ����
* ERROR��  �ȼ�0���ڲ����ش��󣬱���������ʵ�ʳ�������Ѿ��Զ�����
* ALARM��  �ȼ�1���ڲ�һ������ⲿ������ģ����������豸������Ӧ�����������״̬�����ģ����豸��״��
* EVENT��  �ȼ�2���������¼���¼������
* INFOR��  �ȼ�3���¼������Ϣ������ϸ����Ϣ������
* DEBUG��  �ȼ�4��5��6��������Ϣ

��־��Ϣ�ĵȼ�����Ϣ�����һ���ȼ�΢��ֵ��ͬ����������֮�ͣ���΢��ֵ���Ե�����Ϣ��ĵȼ���
�Ը��Ӿ�ȷ�Ĺ��˲�����Ϣ��΢��ֵ���ı���Ϣ������

��ͨ����־������������־��¼ʱ��ʱ������ɹ�������Զ����ϵģ�ֻ��Ҫ���ÿһ����־��Ϣ�
������Ҫ���߹��������һ����־��Ϣ������һ������־��¼�Ŀ�ʼ���һ������־��¼�Ľ�������
ÿ����־��Ϣ��ʱ����ָ����Ϣ����͵ȼ�΢��ֵ��Ĭ��Ϊ0����
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

        // ����Ϣ����͵ȼ�΢��ֵ��ϳ�һ����ֵ���д���
#define MAKE_LOG_LEVEL(level, sub_level) (level | (sub_level << 4))

        class IWriteStream;
        class Logger
        {
        public:
            enum
            {
                kLevelError		= 0,	// ��Ϣ����Ķ���
                kLevelAlarm, 
                kLevelEvent, 
                kLevelInfor, 
                kLevelDebug, 
                kLevelDebug1, 
                kLevelDebug2, 
                kLevelNone, 
                kSubLevel0		= 0x00, // �ȼ�΢��ֵ�Ķ���
                kSubLevel1		= 0x10, 
                kSubLevel2		= 0x20, 
                kSubLevel3		= 0x30, 
                kLogStart		= 0x0100, // ��־��¼��ʼ�ı�־
                kLogEnd			= 0x0200, // ��־��¼�����ı�־
                kLogSingle = kLogStart | kLogEnd, // ������־��Ϣ����ɵ���־��¼�����ǿ�ʼ����ǽ�����
            };

        public:
            Logger();
            ~Logger();

        public:
            // �������ļ���ȡ������Ķ���
            boost::system::error_code load_config(
                framework::configure::Config & conf);

            void pause();

            void resume();

            void log_sec_beg() {++sec_ver_;}

            void log_sec_end() {++sec_ver_;}

            // ����һ�������
            bool add_stream(
                IWriteStream * os );

            // ɾ��һ�������
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
                if ( !( &module ) ) return;/// ��ֹ�ڶ��̻߳����£�һ��ģ�黹δע�ᣬ��һ���̷߳��ʳ���
                if (level + module.level > module.logger.max_level_ || level >= kLevelNone) {
                    return;
                }
                module.logger.private_log(module, level, record);
            }

            // ���һ�����ýṹ����Ϣ��ʽ����־��Ϣ
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

            // ���һ�����ݽϳ�����־��Ϣ�������Ϣ���ݿ��ܳ���1024�ֽڣ�ʹ�ô˽ӿڣ�����ᱻ�м�ض�
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

            // ���һ������16������Ϣ��ʽ����־��Ϣ
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

            // ����һ�������
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

            // ���һ�����ýṹ����Ϣ��ʽ����־��Ϣ
            void private_print_struct(
                LogModule const & module, 
                size_t level, 
                char const * title, 
                char const * fmt, 
                void const * var);

            // ���һ�����ݽϳ�����־��Ϣ�������Ϣ���ݿ��ܳ���1024�ֽڣ�ʹ�ô˽ӿڣ�����ᱻ�м�ض�
            void private_print_string(
                LogModule const & module, 
                size_t level, 
                char const * txt);

            void private_print_string(
                LogModule const & module, 
                size_t level, 
                char const * txt, 
                size_t len);

            // ���һ������16������Ϣ��ʽ����־��Ϣ
            void private_print_hex(
                LogModule const & module, 
                size_t level, 
                unsigned char const * data, 
                size_t len);

        private:
            /// �߳���
            class Lock;
            Lock * lock_;

            /// ���쿪ʼʱ��
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
