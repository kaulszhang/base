// ResolverService.cpp

#include "framework/Framework.h"
#include "framework/logger/Logger.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
#include "framework/string/FormatStl.h"
#include "framework/logger/LoggerFormatRecord.h"
#include "framework/network/ResolverService.h"
#include "framework/network/ResolverIterator.h"
#include "framework/filesystem/Path.h"
#include "framework/timer/ClockTime.h"
#include "framework/network/detail/HostCache.h"
#include "framework/network/detail/ServiceCache.h"
using namespace framework::logger;
using namespace framework::string;
using namespace framework::timer;

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ResolverService", 2);

namespace framework
{
    namespace network
    {

        typedef std::vector<Endpoint> EndpointList;

        struct ResolveTask
        {
        public:
            enum StateEnum {
                stopped, 
                waiting,
                canceled,
                finished,
            };

        public:
            ResolveTask()
                : state(stopped)
                , handler(NULL)
            {
            }

            NetName name;
            StateEnum state;
            std::vector<Endpoint> endpoints;
            boost::system::error_code ec;
            ResolveHandler * handler;
        };

        class ResolverServiceImpl
        {
        public:
            typedef ResolverService::implementation_type implementation_type;

        public:
            ResolverServiceImpl(
                ResolverService & svc)
                : svc_(svc)
                , sync_data_(new sync_data)
                , host_cache_(svc.get_io_service())
                , svc_cache_(svc.get_io_service())
            {
                work_thread_ = new boost::thread(
                    boost::bind(&ResolverServiceImpl::work_thread, this, sync_data_));
            }

            void get_resolvered_host_list(std::string& hosts)
            {
                host_cache_.get_resolvered_host_list(hosts);
            }

            ~ResolverServiceImpl()
            {
                boost::mutex::scoped_lock lock(sync_data_->mtx);
                sync_data_->stop = true;
                sync_data_->cond.notify_one();
                work_thread_->detach();
                if (work_thread_) {
                    delete work_thread_;
                    work_thread_ = NULL;
                }
                // let work_thread delete sync_data_ later
                sync_data_ = NULL;
            }

        public:
            void construct(
                implementation_type & impl)
            {
                impl.reset(new ResolveTask);
            }

            void destroy(
                implementation_type & impl)
            {
                error_code ec;
                cancel(impl, ec);
                impl.reset();
            }

            ResolverIterator resolve(
                implementation_type & impl, 
                NetName const & name, 
                error_code & ec)
            {
                boost::mutex::scoped_lock lock(sync_data_->mtx);
                LOG_F(Logger::kLevelDebug, "[resolve] (name = %1%)" 
                    % name.to_string());
                resolve_no_block(impl, name, ec);
                if (ec == boost::asio::error::would_block) {
                    while (impl->state == ResolveTask::waiting) {
                        sync_data_->cond.wait(lock);
                    }
                    ec = impl->ec;
                }
                if (!ec) {
                    LOG_F(Logger::kLevelDebug, "[resolve] resolved (name = %1%, endpoints = %2%)" 
                        % name.to_string() % format(impl->endpoints));
                    return ResolverIterator(svc_, impl, impl->endpoints[0]);
                } else {
                    LOG_F(Logger::kLevelAlarm, "[resolve] resolved (name = %1%, ec = %2%)" 
                        % name.to_string() % ec.message());
                    return ResolverIterator();
                }
            }

            void async_resolve(
                implementation_type & impl,
                NetName const & name,
                ResolveHandler * handler)
            {
                boost::mutex::scoped_lock lock(sync_data_->mtx);
                LOG_F(Logger::kLevelDebug, "[async_resolve] (name = %1%)" 
                    % name.to_string());
                impl->handler = handler;
                error_code ec;
                resolve_no_block(impl, name, ec);
                if (ec != boost::asio::error::would_block) {
                    call_back(impl, ec);
                }
            }

            void insert_name( 
                NetName const & name, 
                std::vector<Endpoint> const & vec_endpoint)
            {
                host_cache_.update_weak(name, vec_endpoint);
            }

            void increment(
                ResolverIterator & iter)
            {
                implementation_type & impl = iter.impl_;
                boost::mutex::scoped_lock lock(sync_data_->mtx);
                if (iter.current_ < impl->endpoints.size()) {
                    iter.endpoint_ = impl->endpoints[iter.current_];
                    ++iter.current_;
                } else {
                    /* 不等待了，直接结束
                    while (impl->state == ResolveTask::waiting) {
                        sync_data_->cond.wait(lock);
                    }
                    if (iter.current_ < impl->endpoints.size()) {
                        iter.endpoint_ = impl->endpoints[iter.current_];
                        ++iter.current_;
                    } else {
                        iter.current_ = size_t(-1);
                    }
                    */
                    if (impl->state == ResolveTask::waiting) {
                        impl->state = ResolveTask::canceled;
                        impl->ec == boost::asio::error::operation_aborted;
                    }
                    iter.current_ = size_t(-1);
                }
            }

            error_code cancel(
                implementation_type & impl, 
                error_code & ec)
            {
                boost::mutex::scoped_lock lock(sync_data_->mtx);
                if (impl->state == ResolveTask::waiting) {
                    impl->state = ResolveTask::canceled;
                    impl->ec = boost::asio::error::operation_aborted;
                    sync_data_->cond.notify_one();
                    if (impl->handler) {
                        call_back(impl, impl->ec);
                    }
                }
                ec.clear();
                return ec;
            }

        private:
            struct sync_data;

            void work_thread(sync_data * sync_data_)
            {
                boost::posix_time::seconds const delay(5);
                boost::mutex::scoped_lock lock(sync_data_->mtx);
                while (!sync_data_->stop) {
                    if (tasks_.empty() && tasks2_.empty()) {
                        boost::system_time const start = boost::get_system_time();
                        boost::system_time const timeout = start + delay;
                        sync_data_->cond.timed_wait(lock, timeout);
                        // 检查用没有超时的域名
                        if (sync_data_->stop)
                            break;
                        check_host_cache();
                        continue;
                    }

                    implementation_type impl;
                    if (!tasks_.empty()) {
                        impl = tasks_.front();
                    } else {
                        impl = tasks2_.front();
                    }

                    if (impl->state != ResolveTask::waiting) {
                        if (!tasks_.empty() && impl == tasks_.front()) {
                            tasks_.pop_front();
                        } else {
                            tasks2_.pop_front();
                        }
                        continue;
                    } else {
                        boost::system::error_code ec;
                        lock.unlock();
                        EndpointList endpoints;
                        host_cache_.resolve(impl->name, endpoints, ec);
                        lock.lock();
                        if (sync_data_->stop)
                            break;
                        if (!ec)
                            host_cache_.update(impl->name, endpoints);
                        LOG_F(Logger::kLevelDebug1, "[work_thread] resolved (name = %1%, endpoints = %2%)" 
                            % impl->name.to_string() % format(endpoints));
                        std::vector<implementation_type> call_back_tasks;
                        update(impl->name, ec, endpoints, tasks_, call_back_tasks);
                        update(impl->name, ec, endpoints, tasks2_, call_back_tasks);
                        sync_data_->cond.notify_all();
                        lock.unlock();
                        call_back(call_back_tasks);
                        lock.lock();
                    }
                }
                lock.unlock();
                delete sync_data_;
            }

            void resolve_no_block(
                implementation_type & impl, 
                NetName const & name, 
                error_code & ec)
            {
                Endpoint svc;
                svc_cache_.find(name, svc, ec);
                if (ec) {
                    impl->ec = ec;
                    impl->state = ResolveTask::finished;
                    return;
                }
                // 可能service只有一种
                impl->name = name;
                impl->name.protocol((NetName::ProtocolEnum)svc.protocol());
                impl->endpoints.clear();
                impl->ec.clear();
                bool found = host_cache_.find(impl->name, impl->endpoints);
                if (found) {
                    for (size_t i = 0; i < impl->endpoints.size(); ++i) {
                        impl->endpoints[i].port(svc.port());
                    }
                } else {
                    ec = boost::asio::error::would_block;
                }
#ifndef FRAMEWORK_NETWORK_WITH_SERVICE_CACHE
                impl->name.svc(format(svc.port()));
#endif
                impl->state = ResolveTask::waiting;
                tasks_.push_back(impl);
                if (tasks_.size() == 1 && tasks2_.empty()) {
                    sync_data_->cond.notify_one();
                }
            }

            void update(
                NetName const & name, 
                error_code const & ec, 
                EndpointList const & endpoints, 
                std::list<implementation_type> & tasks, 
                std::vector<implementation_type> & call_back_tasks)
            {
                if (!ec) {
                    std::list<implementation_type>::iterator iter = tasks.begin();
                    for (; iter != tasks.end(); ) {
                        implementation_type impl = (*iter);
                        if (impl->name == name) {
                            if (impl->state == ResolveTask::waiting) {
                                impl->state = ResolveTask::finished;
                                LOG_F(Logger::kLevelDebug1, "[update] merge (name = %1%, endpoints = %2%)" 
                                    % impl->name.to_string() % format(impl->endpoints));
                                merge_endpoints(impl->endpoints, endpoints);
                                if (impl->handler) {
                                    call_back_tasks.push_back(impl);
                                }
                            }
                            tasks.erase(iter++);
                        } else {
                            ++iter;
                        }
                    }
                } else {
                    std::list<implementation_type>::iterator iter = tasks.begin();
                    for (; iter != tasks.end(); ) {
                        implementation_type impl = (*iter);
                        if (impl->name == name) {
                            if (impl->state == ResolveTask::waiting) {
                                impl->state = ResolveTask::finished;
                                impl->ec = ec;
                                if (impl->handler) {
                                    call_back_tasks.push_back(impl);
                                }
                            }
                            tasks.erase(iter++);
                        } else {
                            ++iter;
                        }
                    }
                }
            }

            void check_host_cache()
            {
                std::vector<NetName> expired_hosts;
                host_cache_.check(expired_hosts);
                for (size_t i = 0; i < expired_hosts.size(); ++i) {
                    implementation_type impl(new ResolveTask);
                    impl->name = expired_hosts[i];
                    tasks2_.push_back(impl);
                }
            }

            void call_back(
                implementation_type const & impl, 
                error_code const & ec)
            {
                ResolveHandler * handler = impl->handler;
                impl->handler = NULL;
                if (ec) {
                    LOG_F(Logger::kLevelEvent, "[call_back] failed (name = %1%, ec = %2%)" 
                        % impl->name.to_string() % ec.message());
                    svc_.call_back(handler, ec, ResolverIterator());
                } else {
                    LOG_F(Logger::kLevelDebug, "[call_back] resolved (name = %1%, endpoints = %2%)" 
                        % impl->name.to_string() % format(impl->endpoints));
                    svc_.call_back(handler, ec, ResolverIterator(svc_, impl, impl->endpoints[0]));
                }
            }

            void call_back(
                std::vector<implementation_type> const & tasks)
            {
                for (size_t i = 0; i < tasks.size(); ++i) {
                    call_back(tasks[i], tasks[i]->ec);
                }
            }

            void merge_endpoints(
                EndpointList & to, 
                EndpointList const & from)
            {
                size_t n = to.size();
                for (size_t i = 0; i < from.size(); ++i) {
                    EndpointList::iterator end = to.begin() + n;
                    if (std::find(to.begin(), end, from[i]) == end) {
                        to.push_back(from[i]);
                    }
                }
            }

        private:
            struct sync_data
            {
                sync_data()
                    : stop(false)
                {
                }
                boost::mutex mtx;
                boost::condition_variable cond;
                volatile bool stop;
            };

        private:
            ResolverService & svc_;
            boost::thread * work_thread_;
            sync_data * sync_data_;
            std::list<implementation_type> tasks_;
            std::list<implementation_type> tasks2_;
            detail::HostCache host_cache_;
            detail::ServiceCache svc_cache_;
        };

        ResolverService::ResolverService(
            boost::asio::io_service & io_svc)
            : boost::asio::detail::service_base<ResolverService>(io_svc)
            , service_impl_(new ResolverServiceImpl(*this))
        {
        }

        ResolverService::~ResolverService()
        {
            shutdown_service();
        }


        void ResolverService::get_resolvered_host_list(std::string& hosts)
        {
            if (NULL != service_impl_)
                service_impl_->get_resolvered_host_list(hosts);
        }
        
        void ResolverService::shutdown_service()
        {
            if (service_impl_) {
                delete service_impl_;
                service_impl_ = NULL;
            }
        }

        void ResolverService::construct(
            implementation_type & impl)
        {
            service_impl_->construct(impl);
        }

        void ResolverService::destroy(
            implementation_type & impl)
        {
            service_impl_->destroy(impl);
        }

        ResolverIterator ResolverService::resolve(
            implementation_type & impl,
            NetName const & name,
            error_code & ec)
        {
            return service_impl_->resolve(impl, name, ec);
        }

        void ResolverService::pri_async_resolve(
            implementation_type & impl,
            NetName const & name,
            ResolveHandler * handler)
        {
            service_impl_->async_resolve(impl, name, handler);
        }

        void ResolverService::insert_name(
            NetName const & name,
            std::vector<Endpoint> const & vec_endpoint )
        {
            return service_impl_->insert_name( name, vec_endpoint );
        }

        void ResolverService::call_back(
            ResolveHandler * handler, 
            boost::system::error_code const & ec, 
            ResolverIterator const & iter)
        {
            get_io_service().post(
                boost::bind(&ResolveHandler::handle, handler, ec, iter));
        }

        void ResolverService::increment(
            ResolverIterator & iter)
        {
            service_impl_->increment(iter);
        }

        error_code ResolverService::cancel(
            implementation_type & impl, 
            error_code & ec)
        {
            return service_impl_->cancel(impl, ec);
        }

    } // namespace network
} // namespace framework



