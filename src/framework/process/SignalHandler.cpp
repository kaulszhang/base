// SignalHandler.cpp

#include "framework/Framework.h"
#include "framework/process/SignalHandler.h"

#include <boost/thread/mutex.hpp>
using namespace boost::system;

namespace framework
{
    namespace process
    {

        namespace detail
        {

            struct signal
            {
            public:
                signal(
                    Signal sig)
                    : sig_(sig)
                {
                }

            public:
                void register_signal() const
                {
#ifndef BOOST_WINDOWS_API
                    struct sigaction act;
                    act.sa_sigaction = &signal::sigaction;
                    act.sa_flags = SA_RESTART | SA_SIGINFO;
                    // on mac os / ios , sigemptyset is a mocro
#ifndef sigemptyset
                    ::sigemptyset(&act.sa_mask);
#else
                    sigemptyset(&act.sa_mask);
#endif
                    ::sigaction(sig_.value(), &act, &old_action_);
#endif
                }

                void unregister_signal() const
                {
#ifndef BOOST_WINDOWS_API
                    ::sigaction(sig_.value(), &old_action_, NULL);
#endif
                }

            public:
                friend bool operator<(
                    signal const & l, 
                    signal const & r)
                {
                    return l.sig_ < r.sig_;
                }

            private:
                static void invoke_signal(
                    Signal sig);

#ifndef BOOST_WINDOWS_API
                static void sigaction(
                    int sig, 
                    ::siginfo_t *, 
                    void *)
                {
                    invoke_signal((Signal::Enum)sig);
                }
#endif

            private:
                Signal sig_;
#ifndef BOOST_WINDOWS_API
                mutable struct sigaction old_action_;
#endif
            };

            class signal_handler
            {
            private:
                struct handler_data
                {
                    handler_base const * h_;
                    bool once;

                    friend bool operator==(
                        handler_data const & l, 
                        handler_data const & r)
                    {
                        return l.h_ == r.h_;
                    }
                };

                struct is_once
                {
                    bool operator()(
                        handler_data const & h) const
                    {
                        return h.once;
                    }
                };

                typedef std::map<signal, std::vector<handler_data> > handler_map;

            public:
                void insert(
                    Signal sig, 
                    handler_base const * handler, 
                    bool once)
                {
                    handler_data h = {handler, once};
                    boost::mutex::scoped_lock lock(mutex_);
                    handler_map::iterator iter = handlers_.find(sig);
                    if (iter == handlers_.end()) {
                        iter = handlers_.insert(
                            std::make_pair(signal(sig), std::vector<handler_data>())).first;
                        iter->first.register_signal();
                    }
                    iter->second.push_back(h);
                }

                void remove(
                    Signal sig, 
                    handler_base const * handler)
                {
                    handler_data h = {handler, false};
                    boost::mutex::scoped_lock lock(mutex_);
                    handler_map::iterator iter = handlers_.find(sig);
                    if (iter != handlers_.end()) {
                        std::vector<handler_data> & handlers = iter->second;
                        handlers.erase(
                            std::remove(handlers.begin(), handlers.end(), h), 
                            handlers.end());
                        if (handlers.empty()) {
                            handlers_.erase(iter);
                        }
                    }
                }

                void remove(
                    Signal sig)
                {
                    boost::mutex::scoped_lock lock(mutex_);
                    handlers_.erase(sig);
                }

                void remove_all()
                {
                    boost::mutex::scoped_lock lock(mutex_);
                    handlers_.clear();
                }

                void invoke(
                    Signal sig)
                {
                    boost::mutex::scoped_lock lock(mutex_);
                    handler_map::iterator iter = handlers_.find(sig);
                    if (iter != handlers_.end()) {
                        std::vector<handler_data> & handlers = iter->second;
                        for (std::vector<handler_data>::const_iterator iter = handlers.begin();
                            iter != handlers.end(); ++iter) {
                                (*iter).h_->handle();
                        }
                        handlers.erase(
                            std::remove_if(handlers.begin(), handlers.end(), is_once()), 
                            handlers.end());
                        if (handlers.empty())
                            handlers_.erase(sig);
                    }
                }

            private:
                handler_map handlers_;
                boost::mutex mutex_;
            };

            signal_handler & global_signal_handler_impl()
            {
                static signal_handler impl;
                return impl;
            }

            void signal::invoke_signal(
                Signal sig)
            {
                global_signal_handler_impl().invoke(sig);
            }

        } // namespace detail

        
        SignalHandler::~SignalHandler()
        {
            remove();
            delete handler_;
            handler_ = NULL;
        }

        void SignalHandler::insert(
            bool once)
        {
            detail::global_signal_handler_impl().insert(signal_, handler_, once);
        }

        void SignalHandler::remove()
        {
            detail::global_signal_handler_impl().remove(signal_, handler_);
        }

    } // namespace process
} // namespace framework
