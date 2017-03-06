#ifndef _FRAMEWORK_LOGGER_LOGMSGSTREAM_H_
#define _FRAMEWORK_LOGGER_LOGMSGSTREAM_H_

#include <stdio.h>

#ifdef BOOST_WINDOWS_API
#include <Windows.h>
#endif

namespace framework
{
    namespace logger
    {
        // 输出流接口
        struct IWriteStream
        {
            virtual ~IWriteStream() {}

            virtual void write( char const * logmsg ){ };

            virtual int open( char const * filename, char const * mode ) { return 0; }

            virtual long tell() { return 0; }

            virtual int seek( long offset, int origin ) { return 0; }

            virtual int flush() { return 0; }

        };

#ifdef BOOST_WINDOWS_API
        // 写入DebugPrint
        class DebugStringStream : public IWriteStream
        {
        public:
            virtual void write( char const * logmsg )
            {
                OutputDebugString(logmsg);
            }

            virtual ~DebugStringStream() {}
        };
#endif

        // 写入标准输出
        class OStdStream : public IWriteStream
        {
        public:
            virtual void write( char const * logmsg )
            {
                printf( "%s", logmsg );
            }

            virtual ~OStdStream() {}
        };
#if   defined(__ANDROID__)
#  include <android/log.h>
        class OLOGDStream : public IWriteStream
        {
        public:
            virtual void write(char const * logmsg)
            {
                __android_log_print(ANDROID_LOG_DEBUG, "SINA_SDK_DAC", logmsg);
            }

            virtual ~OLOGDStream() {}
        };
#endif

        // 写入文件
        class OFileStream : public IWriteStream
        {
        public:
            OFileStream()
                : m_filename_( 0 )
                , m_mode_( 0 )
            {
            }

            OFileStream( char const * filename, char const * mode )
                : m_filename_( filename )
                , m_mode_( mode )
            {
                m_filehandle_ = fopen( filename, mode ); 
            }

            virtual int open( char const * filename, char const * mode ) 
            {
                if ( !filename || ! mode ) return 0;
                //if ( m_filehandle_ ) fclose( m_filehandle_ );
                m_filehandle_ = fopen( filename, mode ); 
                m_filename_ = filename;
                m_mode_ = mode;
                return m_filehandle_ == NULL ? 0 : 1;
            }

            virtual void write( char const * logmsg )
            {
                if ( !m_filehandle_ ) return;
                fwrite( logmsg, strlen( logmsg ), 1, m_filehandle_ );
                fflush( m_filehandle_ );
            }

            virtual long tell() 
            { 
                if ( !m_filehandle_ ) return -1;

                return ftell( m_filehandle_ ); 
            }

            virtual int seek( long offset, int origin ) 
            {
                if ( !m_filehandle_ ) return -1;

                return fseek( m_filehandle_, offset, origin ); 
            }

            virtual int flush() 
            { 
                if ( !m_filehandle_ ) return -1;

                return fflush( m_filehandle_ ); 
            }

            virtual ~OFileStream()
            {
                if ( m_filehandle_ ) 
                {
                    flush();
                    fclose( m_filehandle_ );
                }
            }

        private:
            char const * m_filename_, *m_mode_;

            FILE * m_filehandle_;
        };  
    }// namespace logger
}// namespace framework

#endif // _FRAMEWORK_LOGGER_LOGMSGSTREAM_H_
