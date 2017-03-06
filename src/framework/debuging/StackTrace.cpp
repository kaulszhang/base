// StackTrace.cpp

#include "framework/Framework.h"
#include "framework/debuging/BackTrace.h"

#ifndef BOOST_WINDOWS_API

#if (__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 4))

#define NO_INSTRUMENT_FUNCTION __attribute__((__no_instrument_function__))

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>

namespace framework
{
    namespace debuging
    {

        namespace detail
        {

            struct CMemoryObject
            {
                CMemoryObject() NO_INSTRUMENT_FUNCTION;

                ~CMemoryObject() NO_INSTRUMENT_FUNCTION;

                void * operator new(
                    size_t size) throw () NO_INSTRUMENT_FUNCTION;

                void operator delete(
                    void * ptr) throw () NO_INSTRUMENT_FUNCTION;

                void * operator new[](
                    size_t size) throw () NO_INSTRUMENT_FUNCTION;

                void operator delete[](
                    void * ptr) throw () NO_INSTRUMENT_FUNCTION;
            };

            CMemoryObject::CMemoryObject() {}

            CMemoryObject::~CMemoryObject() {}

            void * CMemoryObject::operator new(
                size_t size) throw ()
            {
                return malloc(size);
            }

            void CMemoryObject::operator delete(
                void * ptr) throw ()
            {
                free(ptr);
            }

            void * CMemoryObject::operator new[](
                size_t size) throw ()
            {
                return malloc(size);
            }

            void CMemoryObject::operator delete[](
                void * ptr) throw ()
            {
                free(ptr);
            }

        }

        struct FuncCall
            : detail::CMemoryObject
        {
            FuncCall() NO_INSTRUMENT_FUNCTION;

            ~FuncCall() NO_INSTRUMENT_FUNCTION;

            void * this_func;
            void * call_site;
        };

        FuncCall::FuncCall() {}

        FuncCall::~FuncCall() {}

        struct FuncCallStack
            : detail::CMemoryObject
        {
            FuncCallStack() NO_INSTRUMENT_FUNCTION;

            ~FuncCallStack() NO_INSTRUMENT_FUNCTION;

            void push(
                FuncCall const & fc) NO_INSTRUMENT_FUNCTION;

            void pop(
                FuncCall const & fc) NO_INSTRUMENT_FUNCTION;

            void dump(
                int sig) const NO_INSTRUMENT_FUNCTION;

        private:
            size_t capacity_;
            size_t size_;
            FuncCall * stack_;
        };

        FuncCallStack::FuncCallStack()
        {
            capacity_ = 128;
            stack_ = new FuncCall[capacity_];
            if (stack_ == NULL) {
                capacity_ = 0;
            }
            size_ = 0;
        }

        FuncCallStack::~FuncCallStack()
        {
            if (stack_) {
                delete [] stack_;
                stack_ = NULL;
            }
        }

        void FuncCallStack::push(
            FuncCall const & fc)
        {
            if (size_ == capacity_) {
                FuncCall * new_stack = new FuncCall[capacity_ * 2];
                if (new_stack) {
                    capacity_ = capacity_ * 2;
                    memcpy(new_stack, stack_, sizeof(FuncCall) * size_);
                    if (stack_)
                        delete [] stack_;
                    stack_ = new_stack;
                }
            }
            if (size_ < capacity_) {
                stack_[size_++] = fc;
            }
        }

        void FuncCallStack::pop(
            FuncCall const & fc)
        {
            --size_;
        }

        void FuncCallStack::dump(
            int sig) const
        {
            int fd = ::open("/tmp/err_log2", O_RDWR | O_CREAT | O_TRUNC, 0666);
            ::system("echo -e '*********exit_sighandler*********'\n");
            if (fd < 0) {
                exit(1);
            }
            char str[128];
            ::sprintf(str, "SIG %d in thread 0x%.8x of process %d(%d)\n", sig, (unsigned long)pthread_self(), getpid(), getppid());
            ::write(fd, str, strlen(str));
            void ** addrs = (void **)stack_;
            for (size_t i = 0; i < size_; ++i) {
                addrs[i] = stack_[i].this_func;
            }
            back_trace_symbols_fd(addrs, size_, fd);
            ::close(fd);
            exit(1);
        }

        class StackTrace
        {
        public:
            StackTrace() NO_INSTRUMENT_FUNCTION;

            ~StackTrace() NO_INSTRUMENT_FUNCTION;

            void push(
                void * this_func, 
                void * call_site) NO_INSTRUMENT_FUNCTION;

            void pop(
                void * this_func, 
                void * call_site) NO_INSTRUMENT_FUNCTION;

            void dump(int sig) const NO_INSTRUMENT_FUNCTION;

        private:
            static void destructor(
                void * arg) NO_INSTRUMENT_FUNCTION;

        private:
            FuncCallStack * get_stack() const NO_INSTRUMENT_FUNCTION;

        private:
            bool inited_;
            pthread_key_t tss_key_;
        };

        StackTrace::StackTrace()
        {
            inited_ = ::pthread_key_create(&tss_key_, &StackTrace::destructor) == 0;
        }

        StackTrace::~StackTrace()
        {
            if (inited_)
                ::pthread_key_delete(tss_key_);
        }

        void StackTrace::push(
            void * this_func, 
            void * call_site)
        {
            FuncCall fc;
            fc.this_func = this_func;
            fc.call_site = call_site;
            get_stack()->push(fc);
        }

        void StackTrace::pop(
            void * this_func, 
            void * call_site)
        {
            FuncCall fc;
            fc.this_func = this_func;
            fc.call_site = call_site;
            get_stack()->pop(fc);
        }

        void StackTrace::dump(int sig) const
        {
            get_stack()->dump(sig);
        }

        void StackTrace::destructor(
            void * arg)
        {
            FuncCallStack * stack = (FuncCallStack *)arg;
            delete stack;
        }

        FuncCallStack * StackTrace::get_stack() const
        {
            FuncCallStack * stack = (FuncCallStack *)::pthread_getspecific(tss_key_);
            if (stack == NULL) {
                stack = new FuncCallStack;
                ::pthread_setspecific(tss_key_, stack);
            }
            return stack;
        }

        static StackTrace & NO_INSTRUMENT_FUNCTION get_stack_trace();

        static void NO_INSTRUMENT_FUNCTION exit_sighandler(
            int sig);

        static StackTrace & get_stack_trace()
        {
            static StackTrace stack_trace;
            return stack_trace;
        }

        static void exit_sighandler(
            int sig)
        {
            get_stack_trace().dump(sig);
        }

        void install_stack_trace()
        {    
            signal(SIGINT, exit_sighandler);  // Interrupt from keyboard
            signal(SIGTERM, exit_sighandler); // kill
            signal(SIGBUS, exit_sighandler);  // bus error
            signal(SIGSEGV, exit_sighandler); // segfault
            signal(SIGILL, exit_sighandler);  // illegal instruction
            signal(SIGFPE, exit_sighandler);  // floating point exc.
            signal(SIGABRT, exit_sighandler); // abort()
        }

    } // namespace debuging
} // namespace framework

extern "C"
{

    void __cyg_profile_func_enter(
        void * this_func, 
        void * call_site) NO_INSTRUMENT_FUNCTION;

    void __cyg_profile_func_exit(
        void * this_func, 
        void * call_site) NO_INSTRUMENT_FUNCTION;

    void __cyg_profile_func_enter(
        void * this_func, 
        void * call_site)
    {
        framework::debuging::get_stack_trace().push(this_func, call_site);
    }

    void __cyg_profile_func_exit(
        void * this_func, 
        void * call_site)
    {
        framework::debuging::get_stack_trace().pop(this_func, call_site);
    }

}

#endif

#endif // BOOST_WINDOWS_API
