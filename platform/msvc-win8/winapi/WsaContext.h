// WsaContext.h

#pragma once

#include "SocketEmulation.h"
#include "ThreadEmulation.h"

#include <vector>
#include <deque>
#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <condition_variable>

#include <assert.h>

namespace winapi
{

    struct wsa_handle
        : public boost::enable_shared_from_this<wsa_handle>
    {
        typedef boost::shared_ptr<wsa_handle> pointer_t;

        size_t index;
        size_t cls; // 1 - socket, 2 - iocp
    };

    template <typename T, size_t C>
    struct wsa_handle_t
        : wsa_handle
    {
        typedef boost::shared_ptr<T> pointer_t;

        static size_t const CLS = C;

        boost::shared_ptr<T> shared_from_this()
        {
            return boost::static_pointer_cast<T>(wsa_handle::shared_from_this());
        }

        boost::shared_ptr<T const> shared_from_this() const
        {
            return boost::static_pointer_cast<T const>(wsa_handle::shared_from_this());
        }
    };

    struct wsa_context
    {
        wsa_context()
        {
            handles_.push_back(wsa_handle::pointer_t()); // 0 ÊÇÎÞÐ§¾ä±ú
        }

        ~wsa_context()
        {
        }

        template <typename handle_t>
        boost::shared_ptr<handle_t> alloc()
        {
            std::unique_lock<std::mutex> lc(mutex_);
            wsa_handle::pointer_t handle(new handle_t);
                handle->cls = handle_t::CLS;
            if (free_indexs_.empty()) {
                handle->index = handles_.size();
                handles_.push_back(handle);
            } else {
                handle->index = free_indexs_.front();
                free_indexs_.pop_front();
                assert(handles_[handle->index] == NULL);
                handles_[handle->index] = handle;
            }
            return boost::static_pointer_cast<handle_t>(handle);
        }

        template <typename handle_t>
        void free(
            boost::shared_ptr<handle_t> handle)
        {
            std::unique_lock<std::mutex> lc(mutex_);
            assert(handles_[handle->index]);
            free_indexs_.push_back(handle->index);
            handles_[handle->index].reset();
        }

        template <typename handle_t>
        boost::shared_ptr<handle_t> get(
            size_t index)
        {
            std::unique_lock<std::mutex> lc(mutex_);
            assert(handles_[index]);
            wsa_handle::pointer_t handle = handles_[index];
            assert(handle->cls == handle_t::CLS);
            return boost::static_pointer_cast<handle_t>(handle);
        }

    private:
        std::mutex mutex_;
        std::vector<wsa_handle::pointer_t> handles_;
        std::deque<size_t> free_indexs_;
    };

    inline wsa_context & g_wsa_context()
    {
        static wsa_context context;
        return context;
    }

}
