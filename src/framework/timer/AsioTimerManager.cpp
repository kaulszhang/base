// AsioTimerManager.cpp

#include "framework/Framework.h"
#include "framework/timer/AsioTimerManager.h"
#include "framework/logger/LoggerSection.h"
using namespace framework::logger;

#include <boost/bind.hpp>

namespace framework
{
    namespace timer
    {

        struct noop_deleter { void operator()(void*) {} };

        AsioTimerManager::AsioTimerManager(
            boost::asio::io_service & io_svc, 
            boost::posix_time::time_duration const & interval,
            bool need_skip)
            : TimerQueue(interval, need_skip)
            , clock_timer(io_svc)
        {
        }

        AsioTimerManager::AsioTimerManager(
            boost::asio::io_service & io_svc, 
            Duration const & interval,
            bool need_skip)
            : TimerQueue(interval, need_skip)
            , clock_timer(io_svc)
        {
        }

        AsioTimerManager::~AsioTimerManager()
        {
        }

        void AsioTimerManager::start()
        {
            cancel_token_.reset(static_cast<void*>(0), noop_deleter());
            expires_at(next_tick());
            async_wait(boost::bind(&AsioTimerManager::handle_timer, 
                this, 
                _1, 
                boost::weak_ptr<void>(cancel_token_)));
        }

        void AsioTimerManager::stop()
        {
            cancel_token_.reset(static_cast<void*>(0), noop_deleter());
            boost::system::error_code ec;
            cancel(ec);
        }

        void AsioTimerManager::handle_timer(
            boost::system::error_code const & ec, 
            boost::weak_ptr<void> const & cancel_token)
        {
            LoggerSection ls;

            if (!cancel_token.expired()) {
                handle_tick();
                expires_at(next_tick());
                async_wait(boost::bind(&AsioTimerManager::handle_timer, 
                    this, 
                    _1, 
                    boost::weak_ptr<void>(cancel_token)));
            }
        }

    } // namespace timer
} // namespace framework
