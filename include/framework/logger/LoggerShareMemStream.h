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
                std::string log_level;      // ��־�ȼ�
                std::string log_module;     // ��־ģ��
                std::string log_msg;        // ��־����
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

            // ��������Ч�ֽڵĸ���
            std::size_t size()
            {
                return this->pptr() - this->gptr();
            }

            // ���������ݣ��ɵ��÷���������
            char * data()
            {
                std::size_t datasize = size();
                char *szBuf = new char[datasize + 1];
                memcpy( szBuf, msg_buf_, datasize );
                szBuf[datasize] = '\0';              
                return szBuf;
            }

            // ��������
            bool flush()
            {
                char *buf = data();
                bool needToSend = false;
                std::string text = buf;
                std::size_t p = 0;
                std::string sendstr;

                // ѭ������
                for ( std::string::size_type q = text.find('\n', p); q != std::string::npos; )
                {
                    needToSend = true;
                    LogMsgFormat msgformat;
                    parseLog( text.substr(0, q), msgformat );

                    std::cout << "��������:\n\t" << "����ȼ���" << msgformat.log_level.c_str() \
                        << "\n\t����ģ�飺" << msgformat.log_module.c_str() << "\n\t������Ϣ��" \
                        << msgformat.log_msg.c_str() << "\n";

                    text = text.substr( q + 1, strlen( buf ) );
                    q = text.find('\n', p);                
                }      

                // ��ʣ���ַ��ƶ����ײ�
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
            // д����Ϣ����
            virtual int sync()
            { 
                flush(); 
                return 0;
            }

            // �����������
            virtual int_type overflow( int_type _Meta )
            { 
                if ( flush() )
                {
                    sputc( traits_type::to_char_type( _Meta ) );
                }
                else
                {
                    // ���仺����
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

            // ���ö�д����Ϊ��ʼλ��
            void rewindPos()
            {                
                this->setg( msg_buf_, msg_buf_, msg_buf_ + buf_size_ );
                this->setp( msg_buf_, msg_buf_ + buf_size_ );
                this->pbump( 0 );
                this->gbump( 0 );
            }
            
            // ������־
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
            char * msg_buf_;        // �����׵�ַ
            std::size_t inc_delta_; // ����
            std::size_t buf_size_;  // �����С
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
