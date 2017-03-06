// Base.h

#ifndef _UTIL_STREAM_BASE_H_
#define _UTIL_STREAM_BASE_H_

namespace util
{
    namespace stream
    {

        class Base
        {
        public:
            Base()
                : io_svc_(NULL)
            {
            }

            boost::asio::io_service & get_io_service()
            {
                return *io_svc_;
            }

        protected:
            void set_io_service(
                boost::asio::io_service & io_svc)
            {
                io_svc_ = &io_svc;
            }

        private:
            boost::asio::io_service * io_svc_;
        };

    } // namespace stream
} // namespace util

#endif // _UTIL_STREAM_BASE_H_
