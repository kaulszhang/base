// Process.h

#ifndef _FRAMEWORK_PROCESS_PROCESS_H_
#define _FRAMEWORK_PROCESS_PROCESS_H_

#include "framework/process/SignalHandler.h"

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

namespace framework
{
    namespace process
    {

        namespace detail
        {
            struct process_data_base;
        }

        struct ProcessInfo
        {
            int pid;
            boost::filesystem::path bin_file;
        };

        struct CpuStat
        {
            boost::uint32_t user;
            boost::uint32_t nice;
            boost::uint32_t system;
            boost::uint32_t idle;
            boost::uint32_t iowait;
            boost::uint32_t irq;
            boost::uint32_t softirq;
            boost::uint32_t total;
        };

        struct SystemStat
        {
            CpuStat cpu;
            std::vector<CpuStat> cpus;
        };

        struct ProcessStat
        {
            enum StateEnum
            {
                running,    // R    Running or runnable (on run queue)
                sleep1,     // D    Uninterruptible sleep (usually IO)
                sleep2,     // S    Interruptible sleep (waiting for an event to complete)
                stopped,    // T    Stopped, either by a job control signal or because it is being traced.
                zombie,     // Z    Defunct ("zombie") process, terminated but not reaped by its parent.
                dead,       // X    dead (should never be seen)
            };

            static StateEnum const char_to_state[26]; // start from "A"

            int pid;            // 进程(包括轻量级进程，即线程)号
            std::string name;   // 应用程序或命令的名字
            StateEnum state;    // 任务的状态
            int ppid;           // 父进程ID
            int pgid;           // 线程组号
            int sid;            // 该任务所在的会话组ID
            int tty_nr;         // 该任务的tty终端的设备号
            int tty_pgrp;       // 终端的进程组号
            boost::uint32_t flags;          // 进程标志位
            boost::uint32_t min_flt;        // 该任务不需要从硬盘拷数据而发生的缺页（次缺页）的次数
            boost::uint32_t cmin_flt;       // 累计的该任务的所有的waited-for进程曾经发生的次缺页的次数目
            boost::uint32_t maj_flt;        // 该任务需要从硬盘拷数据而发生的缺页（主缺页）的次数
            boost::uint32_t cmaj_flt;       // 累计的该任务的所有的waited-for进程曾经发生的主缺页的次数目
            boost::uint32_t utime;          // 该任务在用户态运行的时间，单位为jiffies
            boost::uint32_t stime;          // 该任务在核心态运行的时间，单位为jiffies
            boost::uint32_t cutime;         // 累计的该任务的所有的waited-for进程曾经在用户态运行的时间，单位为jiffies
            boost::uint32_t cstime;         // 累计的该任务的所有的waited-for进程曾经在核心态运行的时间，单位为jiffies
            boost::uint32_t priority;       // 任务的动态优先级
            boost::uint32_t nice;           // 任务的静态优先级
            boost::uint32_t num_threads;    // 该任务所在的线程组里线程的个数
            boost::uint32_t it_real_value;  // 由于计时间隔导致的下一个 SIGALRM 发送进程的时延，以 jiffy 为单位.
            boost::uint32_t start_time;     // 该任务启动的时间，单位为jiffies
            boost::uint32_t vsize;          // 该任务的虚拟地址空间大小（page）
            boost::uint32_t rss;            // 该任务当前驻留物理地址空间的大小(page)
            boost::uint32_t rlim;           // 该任务能驻留物理地址空间的最大值（bytes）
            boost::uint32_t start_code;     // 该任务在虚拟地址空间的代码段的起始地址
            boost::uint32_t end_code;       // 该任务在虚拟地址空间的代码段的结束地址
            boost::uint32_t start_stack;    // 该任务在虚拟地址空间的栈的结束地址
            boost::uint32_t kstkesp;        // esp(32 位堆栈指针) 的当前值, 与在进程的内核堆栈页得到的一致.
            boost::uint32_t kstkeip;        // 指向将要执行的指令的指针, EIP(32 位指令指针)的当前值.
            boost::uint32_t pendingsig;     // 待处理信号的位图，记录发送给进程的普通信号
            boost::uint32_t block_sig;      // 阻塞信号的位图
            boost::uint32_t sigign;         // 忽略的信号的位图
            boost::uint32_t sigcatch;       // 被俘获的信号的位图
            boost::uint32_t wchan;          // 如果该进程是睡眠状态，该值给出调度的调用点
            boost::uint32_t nswap;          // 被swapped的页数，当前没用
            boost::uint32_t cnswap;         // 所有子进程被swapped的页数的和，当前没用
            boost::uint32_t exit_signal;    // 该进程结束时，向父进程所发送的信号
            boost::uint32_t task_cpu;       // 运行在哪个CPU上
            boost::uint32_t task_priority;  // 实时进程的相对优先级别
            boost::uint32_t task_policy;    // 进程的调度策略，0=非实时进程，1=FIFO实时进程；2=RR实时进程 
        };

        struct ProcessStatM
        {
            boost::uint32_t size;       // (pages) 任务虚拟地址空间的大小 VmSize/4
            boost::uint32_t resident;   // (pages) 应用程序正在使用的物理内存的大小 VmRSS/4
            boost::uint32_t shared;     // (pages) 共享页数 0
            boost::uint32_t trs;        // (pages) 程序所拥有的可执行虚拟内存的大小 VmExe/4
            boost::uint32_t lrs;        // (pages) 被映像到任务的虚拟内存空间的库的大小 VmLib/4
            boost::uint32_t drs;        // (pages) 程序数据段和用户态的栈的大小 （VmData+ VmStk ）4
            boost::uint32_t dt;         // (pages) 0 
        };

        boost::system::error_code enum_process(
            boost::filesystem::path const & bin_file, 
            std::vector<ProcessInfo> & processes);

        inline boost::system::error_code enum_process(
            std::vector<ProcessInfo> & processes)
        {
            return enum_process(boost::filesystem::path(), processes);
        }

        boost::system::error_code get_system_stat(
            SystemStat & stat);

        boost::system::error_code get_process_stat(
            int pid, 
            ProcessStat & stat);

        boost::system::error_code get_process_statm(
            int pid, 
            ProcessStatM & statm);

        class Process
            : private boost::noncopyable
        {
        public:
            Process();

            ~Process();

            struct CreateParamter
            {
                CreateParamter()
                    : outfd(-1)
                    , infd(-1)
                    , errfd(-1)
                    , wait(false)
                {
                }

                std::vector<std::string> args;
                int outfd;
                int infd;
                int errfd;
                bool wait;
            };

            boost::system::error_code open(
                boost::filesystem::path const & bin_file, 
                CreateParamter const & paramter, 
                boost::system::error_code & ec);

            boost::system::error_code create(
                boost::filesystem::path const & bin_file, 
                CreateParamter const & paramter, 
                boost::system::error_code & ec);

            boost::system::error_code create(
                boost::filesystem::path const & bin_file, 
                boost::system::error_code & ec);

            boost::system::error_code open(
                boost::filesystem::path const & bin_file, 
                boost::system::error_code & ec);

            boost::system::error_code open(
                int pid, 
                boost::system::error_code & ec);

            bool is_open();

            bool is_alive(
                boost::system::error_code & ec);

            boost::system::error_code join(
                boost::system::error_code & ec);

            boost::system::error_code timed_join(
                unsigned long milliseconds, 
                boost::system::error_code & ec);

            /**
                Send signal to this process.
                It don't work on win32 os
             */
            boost::system::error_code signal(
                Signal sig, 
                boost::system::error_code & ec);

            boost::system::error_code detach(
                boost::system::error_code & ec);

            boost::system::error_code kill(
                boost::system::error_code & ec);

            int exit_code(
                boost::system::error_code & ec);

            boost::system::error_code close(
                boost::system::error_code & ec);

            boost::system::error_code stat(
                ProcessStat & stat) const;

            boost::system::error_code statm(
                ProcessStatM & statm) const;

        public:
            int id() const;

            int parent_id() const;

        public:
            void swap(
                Process & x)
            {
                std::swap(data_, x.data_);
            }

        private:
            detail::process_data_base * data_;
        };

        int parent_id(
            int id);

    } // namespace process

    namespace this_process
    {

        int id();

        int parent_id();

        bool notify_wait(
            boost::system::error_code const & ec);

    } // namespace this_process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_PROCESS_H_
