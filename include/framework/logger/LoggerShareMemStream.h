// LoggerShareMemStream.h

#ifndef _FRAMEWORK_LOGGER_LOGGERSHAREMEMSTREAM_H_
#define _FRAMEWORK_LOGGER_LOGGERSHAREMEMSTREAM_H_

#include <streambuf>
#include <iostream>

namespace framework
{
    namespace logger
    {

        class LogMsgStreambuf 
            : public std::streambuf
        {
            typedef struct _tLogMsgFormat 
            {
                std::string log_level;      // 日志等级
                std::string log_module;     // 日志模块
                std::string log_msg;        // 日志内容
            }LogMsgFormat;

        public:
            LogMsgStreambuf( char * buf, std::size_t bufsize, std::size_t datasize = 0 )
                : msg_buf_( buf )
                , buf_size_( bufsize )
                , inc_delta_( 512 )
            {
                this->setg( msg_buf_, msg_buf_ + datasize, msg_buf_ + bufsize );
                this->setp( msg_buf_, msg_buf_ + bufsize );
                this->pbump( datasize );
            }

            // 缓冲区有效字节的个数
            std::size_t size()
            {
                return this->pptr() - this->gptr();
            }

            // 缓冲区数据，由调用方负责销毁
            char * data()
            {
                std::size_t datasize = size();
                char *szBuf = new char[datasize + 1];
                memcpy( szBuf, msg_buf_, datasize );
                szBuf[datasize] = '\0';              
                return szBuf;
            }

            // 发送数据
            bool flush()
            {
                char *buf = data();
                bool needToSend = false;
                std::string text = buf;
                std::size_t p = 0;
                std::string sendstr;

                // 循环发送
                for ( std::string::size_type q = text.find('\n', p); q != std::string::npos; )
                {
                    needToSend = true;
                    LogMsgFormat msgformat;
                    parseLog( text.substr(0, q), msgformat );

                    std::cout << "发送数据:\n\t" << "错误等级：" << msgformat.log_level.c_str() \
                        << "\n\t出错模块：" << msgformat.log_module.c_str() << "\n\t出错信息：" \
                        << msgformat.log_msg.c_str() << "\n";

                    text = text.substr( q + 1, strlen( buf ) );
                    q = text.find('\n', p);                
                }      

                // 将剩余字符移动到首部
                if ( needToSend )
                {
                    rewindPos();
                    sputn( text.c_str(),text.size() );
                }                  

                delete [] buf;

                return needToSend;
            }

            virtual ~LogMsgStreambuf() { delete [] eback(); }

        protected:
            // 写入消息队列
            virtual int sync()
            { 
                flush(); 
                return 0;
            }

            // 数据溢出调用
            virtual int_type overflow( int_type _Meta )
            { 
                if ( flush() )
                {
                    sputc( traits_type::to_char_type( _Meta ) );
                }
                else
                {
                    // 扩充缓冲区
                    char * szbuf_bak = msg_buf_;
                    std::size_t buflen = size();

                    msg_buf_ = new char[buf_size_ + inc_delta_];
                    if ( !msg_buf_ )
                    {
                        flush();
                        return traits_type::eof();
                    }
                    memset( msg_buf_, 0, buf_size_ + inc_delta_ );
                    memcpy( msg_buf_, szbuf_bak, buflen );
                    memcpy( msg_buf_ + buflen, ( void * )&_Meta, 1 );
                    buf_size_ += inc_delta_;
                    delete [] szbuf_bak;

                    this->setg( msg_buf_, msg_buf_, msg_buf_ + buf_size_ );
                    this->setp( msg_buf_, msg_buf_ + buf_size_ );
                    this->pbump( buflen + 1 );
                    this->gbump( 0 );

                    flush();
                }

                return traits_type::not_eof( _Meta );
            }

            // 设置读写缓冲为起始位置
            void rewindPos()
            {                
                this->setg( msg_buf_, msg_buf_, msg_buf_ + buf_size_ );
                this->setp( msg_buf_, msg_buf_ + buf_size_ );
                this->pbump( 0 );
                this->gbump( 0 );
            }
            
            // 解析日志
            void parseLog( std::string const & log, LogMsgFormat & formatlog )
            {
                if ( log.empty() ) return;

                std::string text = log;                

                std::string::size_type q = text.find( '[' );
                if ( q == std::string::npos ) return;
                std::string::size_type r = text.find( ']', q );
                if ( r == std::string::npos ) return;
                formatlog.log_level = text.substr( q + 1, r - 1 );
                text = text.substr( r + 1, text.size() );

                q = text.find( '[' );
                if ( q == std::string::npos ) return;
                r = text.find( ']', q );
                if ( r == std::string::npos ) return;
                formatlog.log_module = text.substr( q + 1, r - 2 );
                formatlog.log_msg = text.substr( r + 1, text.size() );
            }

        private:
            char * msg_buf_;        // 缓存首地址
            std::size_t inc_delta_; // 增量
            std::size_t buf_size_;  // 缓存大小
        };

        class LogMsgStream : public std::ostream
        {
        public:
            LogMsgStream() : std::ostream( new LogMsgStreambuf( new char[1024], 1024 ) ) {}

            ~LogMsgStream() 
            {
                delete rdbuf();
            }
        };

        static LogMsgStream glogMsgStream;

    } // namespace logger
} // namespace framework

#endif
