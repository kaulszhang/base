// SignalHandler.h

#ifndef _FRAMEWORK_PROCESS_SIGNAL_HANDLER_H_
#define _FRAMEWORK_PROCESS_SIGNAL_HANDLER_H_

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#ifndef BOOST_WINDOWS_API
#  include <signal.h>
#else
#endif

namespace framework
{
    namespace process
    {

        struct Signal
        {
#ifndef BOOST_WINDOWS_API
            enum Enum
            {
                sig_int = SIGINT, 
            };
#else
            enum Enum
            {
                sig_int, 
            };
#endif

            Signal(
                Enum e)
                : e_(e)
            {
            }

            friend bool operator<(
                Signal const & l, 
                Signal const & r)
            {
                return l.e_ < r.e_;
            }

            Enum value() const
            {
                return e_;
            }

        private:
            Enum e_;
        };

        namespace detail
        {

            class handler_base
            {
            protected:
                handler_base()
                {
                }

            public:
                virtual ~handler_base()
                {
                }

                virtual void handle() const = 0;
            };

            template <
                typename Handler
            >
            class handler
                : public handler_base
            {
            public:
                handler(
                    Handler const & h)
                    : h_(h)
                {
                }

            private:
                virtual void handle() const
                {
                    return h_();
                }

            private:
                Handler h_;
            };

        } // namespace detail

        class SignalHandler
            : private boost::noncopyable
        {
        public:
            template <
                typename Handler
            >
            SignalHandler(
                Signal sig, 
                Handler const & handler, 
                bool once = false)
                : signal_(sig)
                , handler_(new detail::handler<Handler>(handler))
            {
                insert(once);
            }

            ~SignalHandler();

        private:
            void insert(
                bool once);

            void remove();

        private:
            Signal signal_;
            detail::handler_base * handler_;
        };

    } // namespace process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_SIGNAL_HANDLER_H_
