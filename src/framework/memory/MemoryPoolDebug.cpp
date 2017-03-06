// MemoryPoolDebug.cpp

#include "framework/Framework.h"
#include "framework/system/BytesOrder.h"
#include "framework/memory/MemoryPoolDebug.h"
#include "framework/container/Ordered.h"
#include "framework/memory/detail/PoolObject.h"
#include "framework/memory/detail/SymbolBuffer.h"
#include "framework/debuging/BackTrace.h"

#ifdef BOOST_WINDOWS_API
#include <Windows.h>
#endif

namespace framework
{
    namespace memory
    {

        struct MemoryPoolDebug::DebugStack
            : detail::PoolObject
        {
        public:
            BOOST_STATIC_CONSTANT(size_t, depth = 6);

            DebugStack()
                : num_(0)
                , lines_(NULL)
                , obj_num_(0)
            {
                for (size_t i = 0; i < depth; ++i) {
                    addrs_[i] = NULL;
                }
            }

            DebugStack(
                void ** raddrs, 
                size_t num)
                : num_(num)
                , lines_(NULL)
                , obj_num_(0)
            {
                for (size_t i = 0; i < depth; ++i) {
                    addrs_[i] = raddrs[i];
                }
            }

            bool operator<(
                DebugStack const & r) const
            {
                for (size_t i = 0; i < num_ && i < r.num_; ++i) {
                    if (addrs_[i] != r.addrs_[i])
                        return addrs_[i] < r.addrs_[i];
                }
                return num_ < r.num_;
            }

            size_t size() const
            {
                return num_;
            }

            void lines(
                char ** ls)
            {
                lines_ = ls;
            }

            char ** lines() const
            {
                return lines_;
            }

            size_t common_size(
                void * const * raddrs, 
                size_t n) const
            {
                if (n > num_)
                    n = num_;
                for (size_t i = 0; i < n; ++i) {
                    if (addrs_[i] != raddrs[i]) {
                        return i;
                    }
                }
                return n;
            }

            void shift(
                DebugStack const & r, 
                size_t n1, // new common size
                size_t n2) // shift size
            {
                size_t end = num_;
                if (end + n2 > depth) {
                    end = depth - n2;
                }
                for (size_t i = end - 1; i != size_t(-1); --i) {
                    addrs_[i + n2] = addrs_[i];
                }
                for (size_t i = 0; i < n2; ++i) {
                    addrs_[i] = r.addrs_[i + n1];
                }
                num_ = end + n2;
            }

            void drop(
                size_t n2) // shift size
            {
                num_ -= n2;
            }

            void mark()
            {
                ++obj_num_;
            }

            void unmark()
            {
                --obj_num_;
            }

            size_t obj_num() const
            {
                return obj_num_;
            }

            void * const * addrs() const
            {
                return addrs_;
            }

        private:
            void * addrs_[depth];
            size_t num_;
            char ** lines_;
            size_t obj_num_;
        };

        struct MemoryPoolDebug::DebugStackItem
            : framework::container::OrderedHook<DebugStackItem>::type
            , DebugStack
        {
            DebugStackItem(
                DebugStack const & ds)
                : DebugStack(ds)
            {
            }
        };

        struct MemoryPoolDebug::DebugStackSet
            : framework::container::Ordered<DebugStackItem>
            , detail::PoolObject
        {
        };

        MemoryPoolDebug::MemoryPoolDebug()
        {
            framework::debuging::back_trace_init();
            cds_ = new DebugStack;
            dss_ = new DebugStackSet;
        }

        MemoryPoolDebug::MemoryPoolDebug(
            MemoryPoolDebug const & r)
        {
            cds_ = new DebugStack;
            dss_ = new DebugStackSet;
        }

        MemoryPoolDebug::~MemoryPoolDebug()
        {
            if (dss_) {
                while (DebugStackItem * dsi = dss_->first()) {
                    dss_->erase(dsi);
                    if (dsi->lines()) {
                        framework::debuging::release_symbols(dsi->lines());
                    }
                    delete dsi;
                }
                delete dss_;
            }
            if (cds_) {
                if (cds_->lines()) {
                    framework::debuging::release_symbols(cds_->lines());
                }
                delete cds_;
            }
        }

        void MemoryPoolDebug::mark(
            DebugInfo & info)
        {
            void * addr[DebugStack::depth * 2];
            size_t num = dss_->empty() ? DebugStack::depth * 2 : cds_->size() + DebugStack::depth; 
            num = framework::debuging::back_trace(addr, num);
            if (dss_->empty()) {
                *cds_ = DebugStack(addr, num > DebugStack::depth ? DebugStack::depth : num);
            }
            info.stack = push(addr, num);
            info.stack->mark();
        }

        void MemoryPoolDebug::unmark(
            DebugInfo & info)
        {
            info.stack->unmark();
        }

        void MemoryPoolDebug::dump()
        {
            for (DebugStackItem * dsi = dss_->first(); dsi; dsi = dss_->next(dsi)) {
                if (dsi->obj_num()) {
                    output_title(dsi->obj_num());
                    output_symbols(dsi);
                }
            }
        }

        void MemoryPoolDebug::dump(
            DebugInfo const & info, 
            void * addr, 
            size_t size)
        {
            output_title(addr, size);
            output_memory(addr, size);
            if (info.stack == NULL) {
                return;
            }
            output_symbols(info.stack);
        }

        MemoryPoolDebug::DebugStack * MemoryPoolDebug::push(
            void ** addrs, 
            size_t num)
        {
            size_t cz = cds_->common_size(addrs, num);
            if (cz != cds_->size()) {
                size_t sz = cds_->size() - cz;
                for (DebugStackItem * dsi = dss_->first(); dsi; dsi = dss_->next(dsi)) {
                    dsi->shift(*cds_, cz, sz);
                }
                cds_->drop(sz);
                DebugStackItem * last_dsi = NULL;
                for (DebugStackItem * dsi = dss_->first(); dsi; ) {
                    if (last_dsi && !(*last_dsi < *dsi)) {
                        assert(!(*dsi < *last_dsi));
                        DebugStackItem * dsi1 = dsi;
                        dsi = dss_->erase(dsi);
                        delete dsi1;
                    } else {
                        last_dsi = dsi;
                        dsi = dss_->next(dsi);
                    }
                }
            }
            DebugStack ds(addrs + cds_->size(), num - cds_->size());
            framework::container::Ordered<DebugStackItem>::iterator iter = 
                dss_->lower_bound(ds);
            if (iter == dss_->end() || ds < (DebugStack &)*iter) {
                DebugStackItem * dsi = new DebugStackItem(ds);
                iter = dss_->insert(iter, dsi).first;
            }
            return (DebugStack *)&*iter;
        }

        void MemoryPoolDebug::output_title(
            size_t num)
        {
            detail::SymbolBuffer buffer(1);
            buffer.next();
            buffer.push("[MemeryPoolDebug] Memory object (count = ");
            buffer.push(num);
            buffer.push(')');
            char ** lines = buffer.detach();
            output(lines[0]);
            // free
            detail::SymbolBuffer buffer1(lines);
        }

        void MemoryPoolDebug::output_title(
            void * addr, 
            size_t size)
        {
            detail::SymbolBuffer buffer(1);
            buffer.next();
            buffer.push("[MemeryPoolDebug] Memory leak detect at 0x");
            size_t addr1 = system::BytesOrder::host_to_big_endian_long((size_t)addr);
            buffer.push_hex(&addr1, sizeof(addr1), 0);
            buffer.push('(');
            buffer.push(size);
            buffer.push(')');
            char ** lines = buffer.detach();
            output(lines[0]);
            // free
            detail::SymbolBuffer buffer1(lines);
        }

        void MemoryPoolDebug::output_memory(
            void * addr, 
            size_t size)
        {
            for (size_t i = 0; i < size; i += 16) {
                detail::SymbolBuffer buffer(1);
                buffer.next();
                buffer.push_hex(
                    (char *)addr + i, 
                    i + 16 < size ? 16 : size - i, 
                    ' ');
                char ** lines = buffer.detach();
                output(lines[0]);
                // free
                detail::SymbolBuffer buffer1(lines);
            }
        }

        void MemoryPoolDebug::output_symbols(
            DebugStack * ds)
        {
            if (cds_->lines() == NULL && cds_->size()) {
                cds_->lines(framework::debuging::back_trace_symbols(cds_->addrs(), cds_->size()));
            }
            if (ds->lines() == NULL && ds->size()) {
                ds->lines(framework::debuging::back_trace_symbols(ds->addrs(), ds->size()));
            }
            for (size_t i = 0; i < cds_->size(); ++i) {
                output(cds_->lines()[i]);
            }
            for (size_t i = 0; i < ds->size(); ++i) {
                output(ds->lines()[i]);
            }
        }

        void MemoryPoolDebug::output(
            char const * msg)
        {
#ifdef BOOST_WINDOWS_API
            OutputDebugString(msg);
#else
            write(0, msg, strlen(msg));
            write(0, "\n", 1);
#endif
        }

    } // namespace memory
} // namespace framework
