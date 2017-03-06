// Process.cpp

#include "framework/Framework.h"
#include "framework/system/ErrorCode.h"
#include "framework/process/Process.h"
#include "framework/process/Error.h"
#include "framework/filesystem/Symlink.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
#include "framework/logger/LoggerStreamRecord.h"
using namespace framework::system;
using namespace framework::string;
using namespace framework::process::error;
using framework::logger::Logger;

#include <boost/filesystem/operations.hpp>
#include <boost/thread.hpp>
using namespace boost::filesystem;
using namespace boost::system;

#include <fstream>

#ifdef BOOST_WINDOWS_API
#  include <windows.h>

#if (defined WINRT) || (defined WIN_PHONE)
#  include <Processthreadsapi.h>
#endif

#  ifndef __MINGW32__
#ifndef WIN_PHONE
#    include <winternl.h>
#endif
#  endif
#  ifdef UNDER_CE
#    ifndef STARTF_USESTDHANDLES
#       define STARTF_USESTDHANDLES     0x00000100
#    endif
#    ifndef NORMAL_PRIORITY_CLASS
#       define NORMAL_PRIORITY_CLASS    0x00000020
#    endif
#    ifndef CREATE_NO_WINDOW
#       define CREATE_NO_WINDOW         0x08000000
#    endif
#  else
#    include <fcntl.h>
#    include <io.h>
#if !(defined WIN_PHONE) && !(defined WINRT_ARM)
#    include <psapi.h>
#    pragma comment(lib, "Psapi.lib")
#endif
#  endif
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <sys/wait.h>
#  include <unistd.h>
#  include <fcntl.h>
#  include <signal.h>
#  include <poll.h>
#if defined(__APPLE__)
#  include <sys/sysctl.h>
#  define MAX_PROCESS_RANGE             102400
#  define MAX_PATH_LENGTH               2048
#endif
#endif

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Process", 2);

namespace framework
{
    namespace process
    {

        namespace detail
        {
            size_t format(
                char * buf, 
                boost::uint64_t t)
            {
                char * p = buf;
                while (t > 0) {
                    *p++ = '0' + (t & 0x0000000f);
                    t >>= 4;
                }
                return p - buf;
            }

            boost::uint64_t parse(
                char * buf, 
                size_t size)
            {
                boost::uint64_t t = 0;
                for (size_t i = size - 1; i != (size_t)-1; --i) {
                    t <<= 4;
                    t |= (boost::uint64_t)(buf[i] - '0');
                }
                return t;
            }
        }

        static bool has_process_name(
            path const & bin_file, 
            path const & bin_path)
        {
            return bin_file.empty() 
                || (bin_file.is_complete() 
                    ? bin_path == bin_file 
#ifndef BOOST_WINDOWS_API
                    : bin_path.leaf() == bin_file.leaf());
#else
                    : bin_path.stem() == bin_file.stem());
#endif
        }

#ifndef BOOST_WINDOWS_API

#ifdef __FreeBSD__
#  define PROC_EXE "file"
#else
#  define PROC_EXE "exe"
#endif
        static bool get_process_info(
            ProcessInfo & info, 
            pid_t pid, 
            path const & bin_file, 
            error_code & ec)
        {
            path ph(path("/proc") / format(pid));
            if (!is_symlink(ph) // 过滤/proc/self
                && is_symlink(ph / PROC_EXE)) {
                    info.bin_file = framework::filesystem::read_symlink(ph / PROC_EXE, ec);
                    if (!ec && has_process_name(bin_file, info.bin_file)) {
                        info.pid = (int)(pid);
                        return true;
                    }
            } else {
                ec = not_open;
            }
            return false;
        }

        std::istream & operator>>(
            std::istream & is, 
            CpuStat & stat)
        {
            is >> stat.user
                >> stat.nice
                >> stat.system
                >> stat.idle
                >> stat.iowait
                >> stat.irq
                >> stat.softirq;
            stat.total = stat.user + stat.nice + stat.system
                + stat.idle + stat.iowait + stat.irq + stat.softirq;
            return is;
        }

        ProcessStat::StateEnum const ProcessStat::char_to_state[] = {
            dead, dead, dead, sleep1, dead, dead, dead, // A - G
            dead, dead, dead, dead, dead, dead, dead,   // H - N
            dead, dead, dead, running, sleep2, stopped, // O - T
            dead, dead, dead, dead, dead, zombie,       // U - Z
        };

        std::istream & operator>>(
            std::istream & is, 
            ProcessStat & stat)
        {
            is >> stat.pid;
            is.ignore(1);
            is >> stat.name;
            char state;
            is >> state;
            stat.state = ProcessStat::char_to_state[state - 'A'];
            is >> stat.ppid >> stat.pgid >> stat.sid
                >> stat.tty_nr >> stat.tty_pgrp
                >> stat.flags
                >> stat.min_flt >> stat.cmin_flt >> stat.maj_flt >> stat.cmaj_flt
                >> stat.utime >> stat.stime >> stat.cutime >> stat.cstime
                >> stat.priority >> stat.nice
                >> stat.num_threads
                >> stat.it_real_value
                >> stat.start_time
                >> stat.vsize
                >> stat.rss
                >> stat.rlim
                >> stat.start_code >> stat.end_code
                >> stat.start_stack
                >> stat.kstkesp >> stat.kstkeip
                >> stat.pendingsig >> stat.block_sig >> stat.sigign >> stat.sigcatch
                >> stat.wchan
                >> stat.nswap
                >> stat.cnswap
                >> stat.exit_signal
                >> stat.task_cpu >> stat.task_priority >> stat.task_policy;
            return is;
        }

        std::istream & operator>>(
            std::istream & is, 
            ProcessStatM & statm)
        {
            is >> statm.size
                >> statm.resident
                >> statm.shared
                >> statm.trs
                >> statm.lrs
                >> statm.drs
                >> statm.dt;
            return is;
        }

        error_code get_system_stat(
            SystemStat & stat)
        {
            path ph("/proc/stat");
            std::ifstream ifs(ph.string().c_str());
            std::string title;
            ifs >> title >> stat.cpu;
            while (ifs >> title) {
                if (title.substr(0, 3) == "cpu") {
                    stat.cpus.resize(stat.cpus.size() + 1);
                    ifs >> stat.cpus.back();
                } else {
                    break;
                }
            }
            return error_code();
        }

        boost::system::error_code get_process_stat(
            int pid, 
            ProcessStat & stat)
        {
            path ph(path("/proc") / format(pid) / "stat");
            std::ifstream ifs(ph.string().c_str());
            if (ifs.bad()) {
                return last_system_error();
            }
            ifs >> stat;
            return error_code();
        }

        boost::system::error_code get_process_statm(
            int pid, 
            ProcessStatM & statm)
        {
            path ph(path("/proc") / format(pid) / "statm");
            std::ifstream ifs(ph.string().c_str());
            if (ifs.bad()) {
                return last_system_error();
            }
            ifs >> statm;
            return error_code();
        }

#elif (!defined UNDER_CE)

        static bool get_process_info(
            ProcessInfo & info, 
            DWORD pid, 
            path const & bin_file, 
            error_code & ec)
        {
//modify by zhaoyf 2013-11-29
//在app模式下，OpenProcess系列函数为不可用的，在移动终端上一般不存在打开进程的业务，暂时关闭接口
#if !STREAMSDK_WIN8_APP
            CHAR szName[MAX_PATH] = "";
            HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                FALSE, pid);
            if (hProcess) {
                HMODULE hModule;
                DWORD needed;
                // 枚举当前进程调用的所有模块
                if (::EnumProcessModules(hProcess, &hModule, sizeof(hModule), &needed)) {
                    ::GetModuleBaseName(hProcess, hModule, szName, sizeof(szName));
                    info.bin_file = szName;
                    if (has_process_name(bin_file, info.bin_file)) {
                        info.pid = (int)pid;
                        ::CloseHandle(hProcess);
                        return true;
                    }
                }
                ec = last_system_error();
                ::CloseHandle(hProcess);
                return false;
            }
            ec = last_system_error();
            return false;
#else
            assert(false);
            ec = not_open;
			return false;
#endif
//modify end.
        }

        error_code get_system_stat(
            SystemStat & stat)
        {
//modify by zhaoyf 2013-11-29
//在app模式下，GetSystemTimes接口不支持，暂时屏蔽api
#if !STREAMSDK_WIN8_APP
            FILETIME t[3];
            BOOL ret = ::GetSystemTimes(t, t + 1, t + 2);
            if (ret) {
                stat.cpu.user = t[2].dwLowDateTime;
                stat.cpu.nice = 0;
                stat.cpu.system = t[1].dwLowDateTime;
                stat.cpu.idle = t[0].dwLowDateTime;
                stat.cpu.iowait = 0;
                stat.cpu.irq = 0;
                stat.cpu.softirq = 0;
                stat.cpu.total = stat.cpu.idle + stat.cpu.system + stat.cpu.user;
            }
            return last_system_error();
#else
            assert(false);
            return error_code(not_open);
#endif
//modify end.
        }

        boost::system::error_code get_process_stat(
            HANDLE hp, 
            ProcessStat & stat)
        {
//modify by zhaoyf 2013-11-29
//在app模式下，GetProcessTimes接口不支持，暂时屏蔽api
#if !STREAMSDK_WIN8_APP
            FILETIME t[4];
            BOOL ret = ::GetProcessTimes(
                hp, 
                t, 
                t + 1, 
                t + 2, 
                t + 3);
            if (ret) {
                stat.pid = ::GetProcessId(hp);
                stat.state = ProcessStat::running;
                stat.stime = t[2].dwLowDateTime;
                stat.utime = t[3].dwLowDateTime;
            }
            return last_system_error();
#else
            assert(false);
            return error_code(not_open);
#endif
//modify end.
        }

        boost::system::error_code get_process_statm(
            HANDLE hp, 
            ProcessStatM & statm)
        {
//modify by zhaoyf 2013-11-29
//在app模式下，GetProcessMemoryInfo接口不支持，暂时屏蔽api
#if !STREAMSDK_WIN8_APP
            PROCESS_MEMORY_COUNTERS pmc = {sizeof(pmc)};
            if (::GetProcessMemoryInfo(
                hp, 
                &pmc, 
                sizeof(pmc))) {
                statm.size = pmc.PagefileUsage;
                statm.resident = pmc.WorkingSetSize;
                statm.shared = 0;
                statm.trs = 0;
                statm.lrs = 0;
                statm.drs = 0;
                statm.dt = 0;
            }
            return last_system_error();
#else
            assert(false);
            return error_code(not_open);
#endif
//modify end.
        }

        boost::system::error_code get_process_stat(
            int pid, 
            ProcessStat & stat)
        {
//modify by zhaoyf 2013-11-29
#if !STREAMSDK_WIN8_APP
            HANDLE hProcess = ::OpenProcess(
                PROCESS_ALL_ACCESS, 
                FALSE, 
                pid);
            if (hProcess == NULL) {
                return last_system_error();
            }
            return get_process_stat(hProcess, stat);
#else
            assert(false);
            return boost::system::error_code(not_open);
#endif
//modify end.
        }

        boost::system::error_code get_process_statm(
            int pid, 
            ProcessStatM & statm)
        {
//modify by zhaoyf 2013-11-29
#if !STREAMSDK_WIN8_APP
            HANDLE hProcess = ::OpenProcess(
                PROCESS_ALL_ACCESS, 
                FALSE, 
                pid);
            if (hProcess == NULL) {
                return last_system_error();
            }
            return get_process_statm(hProcess, statm);
#else
            assert(false);
            return boost::system::error_code(not_open);
#endif
//modify end.
        }

#endif

        boost::system::error_code enum_process(
            path const & bin_file, 
            std::vector<ProcessInfo> & processes)
        {
#ifndef BOOST_WINDOWS_API
#if defined (__APPLE__)
            int mib[4];
            size_t i, len;
            struct kinfo_proc kp;

            len = 4;
            if (sysctlnametomib("kern.proc.pid", mib, &len) < 0) {
                return last_system_error();
            }

            for (i = 0; i < MAX_PROCESS_RANGE; i++) {
                mib[3] = i;
                len = sizeof(kp);
                if (sysctl(mib, 4, &kp, &len, NULL, 0) == -1) {
                    return last_system_error();
                } else if (len > 0) {
                    int mib1[4];
                    char *proc_path = NULL;
                    size_t size = MAX_PATH_LENGTH;

                    mib1[0] = CTL_KERN;
                    mib1[1] = KERN_PROCARGS;
                    mib1[2] = kp.kp_proc.p_pid;
                    mib1[3] = 0;

                    proc_path = new char[size * sizeof(char)];
                    if (sysctl(mib1, 3, proc_path, &size, NULL, 0) < 0) {
                        continue;
                    } else {
                        ProcessInfo p_info;
                        p_info.bin_file = proc_path;
                        if (p_info.bin_file.leaf() == bin_file || bin_file.empty()) {
                            p_info.pid =  kp.kp_proc.p_pid;
                            processes.push_back(p_info);
                        }
             // 
               //         printf("pid = %d, ppid = %d, name = %s, path = %s\n", kp.kp_proc.p_pid, kp.kp_eproc.e_ppid, kp.kp_proc.p_comm, proc_path);
                    }
                    delete[] proc_path;
                }
            }
            return error_code();
#else
            path proc_path("/proc");
            if (!exists(proc_path))
                return framework::system::logic_error::not_supported;
            directory_iterator iter(proc_path);
            directory_iterator end;
            for (; iter != end; ++iter) {
                ProcessInfo info;
                error_code ec;
                if (get_process_info(info, parse<pid_t>(iter->path().filename().string()), bin_file, ec)) {
                    processes.push_back(info);
                }
            }
            return error_code();
#endif
#elif (!defined UNDER_CE)
//modify by zhaoyf 2013-11-29
#if !STREAMSDK_WIN8_APP
             DWORD pids[1024], needed;
            if (!::EnumProcesses(pids, sizeof(pids), &needed)) {
                return last_system_error();
            }
            DWORD actualProcessCount = needed / sizeof(DWORD);
            for (DWORD i = 0; i < actualProcessCount; i++) {
                ProcessInfo info;
                error_code ec;
                if (get_process_info(info, pids[i], bin_file, ec)) {
                    processes.push_back(info);
                }
            }
            return error_code();
#else
            //assert(false);
            SetLastError(ERROR_NOT_SUPPORTED);
            return last_system_error();
#endif
//modify end.
           
#else
            SetLastError(ERROR_NOT_SUPPORTED);
            return last_system_error();
#endif
        }

        namespace detail
        {
            struct process_data_base
            {
                bool is_alive;
#ifndef BOOST_WINDOWS_API
                pid_t pid;
                pid_t ppid;
                std::vector<ProcessInfo> * handle;
                int status;
#else
                DWORD pid;
                DWORD ppid;
                HANDLE handle;
                DWORD status;

#endif
                process_data_base()
                    : is_alive(true)
                    , pid(0)
                    , handle(NULL)
                    , status(0)
                {
                }
            };
        }

        Process::Process()
            : data_(NULL)
        {
        }

        Process::~Process()
        {
            if (is_open()) {
                error_code ec;
                close(ec);
            }
        }

#ifdef BOOST_WINDOWS_API
        static HANDLE get_pipe_handle(int fd)
        {
#ifdef UNDER_CE 
            return INVALID_HANDLE_VALUE;
#else
//modify by zhaoyf 2013-11-29
//在app模式下，SetHandleInformation接口不支持，暂时屏蔽api
#if !STREAMSDK_WIN8_APP
            if (-1 == fd)
                return INVALID_HANDLE_VALUE;
            HANDLE h = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
            ::SetHandleInformation(h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            return h;
#else
            assert(false);
            return INVALID_HANDLE_VALUE;
#endif
//modify end.
#endif
        }
#endif

        boost::system::error_code Process::open(
            path const & bin_file, 
            CreateParamter const & paramter, 
            boost::system::error_code & ec)
        {
            if (open(bin_file, ec)) {
                create(bin_file, paramter, ec);
            }
            return ec;
        }

        error_code Process::create(
            path const & bin_file, 
            boost::system::error_code & ec)
        {
            return create(bin_file, CreateParamter(), ec);
        }

        error_code Process::create(
            path const & bin_file, 
            CreateParamter const & paramter, 
            boost::system::error_code & ec)
        {
            if (is_open())
                return ec = already_open;

            std::string bin_file_path = bin_file.string();
#ifndef BOOST_WINDOWS_API
            const char * filename = bin_file_path.c_str();
            std::vector<char *> cmdArr(paramter.args.size() + 2, 0);
            cmdArr[0] = &bin_file_path[0];
            for(size_t i = 0; i < paramter.args.size(); ++i) {
                cmdArr[i + 1] = (char *)paramter.args[i].c_str();    
            }
            int pipefd[2];
            if (paramter.wait) {
                ::pipe(pipefd);
            }
            pid_t pid = fork();
            if (pid < 0) {
                if (paramter.wait) {
                    ::close(pipefd[0]);
                    ::close(pipefd[1]);
                }
                ec = last_system_error();
            } else if (pid == 0) {
                int nfd = ::open("/dev/null", O_RDWR);
                if (paramter.infd == -2)
                    dup2(nfd, STDIN_FILENO);
                else if (paramter.infd != -1)
                    dup2(paramter.infd, STDIN_FILENO);
                if (paramter.outfd == -2)
                    dup2(nfd, STDOUT_FILENO);
                else if (paramter.outfd != -1)
                    dup2(paramter.outfd, STDOUT_FILENO);
                if (paramter.errfd == -2)
                    dup2(nfd, STDERR_FILENO);
                else if (paramter.errfd != -1)
                    dup2(paramter.errfd, STDERR_FILENO);
                ::close(nfd);
                int i = 3;
                if (paramter.wait) {
                    dup2(pipefd[1], 3);
                    i = 4;
                    ::setenv("FRAMEWORK_PROCESS_WAIT_FILE", "3", 1 /* overwrite */);
                }
#define MAXFILE 65535
                for(; i < MAXFILE; ++i)    //关闭父进程打开的文件描述符，主要是为了关闭socket
				{
#ifdef __ANDROID__
					if ( i < 8 || i > 18 )
#endif
                        ::close(i);
				}
#undef MAXFILE
                if (execvp(filename, &cmdArr.at(0)) < 0) {
                    framework::this_process::notify_wait(last_system_error());
                    _exit(errno);
                }
                return error_code();
            } else {
                ec = error_code();
                ProcessInfo pi;
                pi.pid = pid;
                pi.bin_file = bin_file;
                data_ = new detail::process_data_base;
                data_->pid = pid;
                data_->ppid = ::getpid();
                data_->handle = new std::vector<ProcessInfo>(1, pi);
                if (paramter.wait) {
                    ::close(pipefd[1]);
                    char buffer[64];
                    int len = ::read(pipefd[0], buffer, sizeof(buffer));
                    char * p = ::strchr(buffer, ':');
                    if (p) {
                        *p++ = '\0';
                        ec.assign((int)framework::process::detail::parse(p, buffer + len - p), 
                            boost::system::error_category::find_category(buffer));
                    } else {
                        ec = not_alive;
                    }
                    ::close(pipefd[0]);
                }
            }
#else
//modify by zhaoyf 2013-11-29
//在app模式下，STARTUPINFO接口不支持，暂时屏蔽api
#if !STREAMSDK_WIN8_APP
            std::string cmdlinestr;
            if (bin_file_path.find(' ') == std::string::npos) {
                cmdlinestr = bin_file_path;
            } else {
                cmdlinestr = " \"" + bin_file_path + "\"";
            }
            for (size_t i = 0; i < paramter.args.size(); ++i) {
                if (!paramter.args[i].empty() && paramter.args[i].find(' ') == std::string::npos) {
                    cmdlinestr += " " + paramter.args[i];
                } else {
                    cmdlinestr += " \"" + paramter.args[i] + "\"";
                }
            }
            STARTUPINFO si;
            memset(&si, 0, sizeof(si));
            si.cb = sizeof( si );
            bool inheritHandles = false;
            if (-1 != paramter.outfd) {
                si.hStdOutput = get_pipe_handle(paramter.outfd);
                if (INVALID_HANDLE_VALUE != si.hStdOutput) {
                    inheritHandles = true;
                }
                si.dwFlags = STARTF_USESTDHANDLES;
            }
            if (-1 != paramter.infd) {
                si.hStdInput = get_pipe_handle(paramter.infd);
                if (INVALID_HANDLE_VALUE != si.hStdInput) {
                    inheritHandles = true;
                }
                si.dwFlags = STARTF_USESTDHANDLES;
            }
            if (-1 != paramter.errfd) {
                si.hStdError = get_pipe_handle(paramter.errfd);
                if (INVALID_HANDLE_VALUE != si.hStdError) {
                    inheritHandles = true;
                }
                si.dwFlags = STARTF_USESTDHANDLES;
            }
            {
                char pid_buf[32];
                size_t len = 0;
                len = framework::process::detail::format(pid_buf, GetCurrentProcessId());
                pid_buf[len] = '\0';
                ::SetEnvironmentVariable(
                    "FRAMEWORK_PROCESS_PARENT_ID", 
                    pid_buf);
            }
            HANDLE hReadPipe;
            HANDLE hWritePipe;
            if (paramter.wait) {
                ::CreatePipe(
                    &hReadPipe, 
                    &hWritePipe, 
                    NULL, 
                    0);
                char handle_buf[32];
                size_t len = 0;
                len = framework::process::detail::format(handle_buf, (boost::uint64_t)hWritePipe);
                assert((boost::uint64_t)hWritePipe == framework::process::detail::parse(handle_buf, len));
                handle_buf[len] = '\0';
                ::SetEnvironmentVariable(
                    "FRAMEWORK_PROCESS_WAIT_FILE", 
                    handle_buf);
            }
            PROCESS_INFORMATION pi = { 0 };
            if (FALSE == ::CreateProcess(
                NULL, 
                &cmdlinestr[0], 
                NULL, 
                NULL, 
                inheritHandles ? TRUE : FALSE, 
                NORMAL_PRIORITY_CLASS, 
                NULL, 
                NULL, 
                &si, 
                &pi)) {
                    ec = last_system_error();
                    if (paramter.wait) {
                        ::SetEnvironmentVariableA(
                            "FRAMEWORK_PROCESS_WAIT_FILE", 
                            NULL);
                        ::CloseHandle(
                            hReadPipe);
                        ::CloseHandle(
                            hWritePipe);
                    }
            } else {
                if (paramter.wait) {
                    ::SetEnvironmentVariableA(
                        "FRAMEWORK_PROCESS_WAIT_FILE", 
                        NULL);
                    CHAR Buffer[64] = {0};
                    DWORD NumberOfBytesRead = 0;
                    ::ReadFile(
                        hReadPipe, 
                        Buffer, 
                        sizeof(Buffer), 
                        &NumberOfBytesRead, 
                        NULL);
                    CHAR * p = (char *)::memchr(Buffer, ':', NumberOfBytesRead);
                    if (p) {
                        *p++ = '\0';
                        ec.assign((int)(detail::parse(p, Buffer + NumberOfBytesRead - p)), 
                            boost::system::error_category::find_category(Buffer));
                    } else {
                        ec = not_alive;
                    }
                    ::CloseHandle(
                        hWritePipe);
                    ::CloseHandle(
                        hReadPipe);
                }
                data_ = new detail::process_data_base;
                data_->pid = pi.dwProcessId;
                data_->ppid = ::GetCurrentProcessId();
                data_->handle = pi.hProcess;
                ::CloseHandle(
                    pi.hThread);
                ec = error_code();
            }
#else
            //assert(false);
            ec = error_code();
#endif
#endif
            LOG_S(Logger::kLevelDebug, "create " 
                << bin_file.string() 
                << " " << (data_ ? data_->pid : 0) 
                << " " << ec.message());
            return ec;
        }

        error_code Process::open(
            path const & bin_file, 
            error_code & ec)
        {
            if (is_open())
                return ec = already_open;

            std::vector<ProcessInfo> pis;
            ec = enum_process(bin_file, pis);
            if (!ec) {
                if (pis.empty()) {
                    ec = not_open;
                } else {
#ifndef BOOST_WINDOWS_API
                    data_ = new detail::process_data_base;
                    data_->pid = pis[0].pid;
                    data_->ppid = 0;
                    data_->handle = new std::vector<ProcessInfo>;
                    for (size_t i = 0; i < pis.size(); ++i) {
                        data_->handle->push_back(pis[i]);
                    }
#else
                    open(pis[0].pid, ec);
#endif
                }
            }
            LOG_S(Logger::kLevelDebug, "open " << bin_file.string() 
                << " " << (data_ ? data_->pid : 0) 
                << " " << ec.message());
            return ec;
        }

        error_code Process::open(
            int pid, 
            error_code & ec)
        {
            if (is_open())
                return ec = already_open;

            data_ = new detail::process_data_base;
            data_->pid = pid;
            data_->ppid = 0;
#ifndef BOOST_WINDOWS_API
            ProcessInfo info;
            if (get_process_info(info, pid, "", ec)) {
                data_->handle = new std::vector<ProcessInfo>(1, info);
            } else {
                LOG_S(Logger::kLevelDebug, "open (no such process): " << ec.message());
            }
#else       
//modify by zhaoyf 2013-11-29
//在app模式下，GetSystemTimes接口不支持，暂时屏蔽api
#if !STREAMSDK_WIN8_APP
            data_->handle = ::OpenProcess(
                PROCESS_ALL_ACCESS, 
                FALSE, 
                pid);
            if (!data_->handle) {
                ec = last_system_error();
                delete data_;
                data_ = NULL;
            }
#else
            //assert(false);
            return error_code(not_open);
#endif
//modify end.
#endif
            return ec;
        }

        bool Process::is_open()
        {
            return !(data_ == NULL);
        }

        bool Process::is_alive(
            error_code & ec)
        {
            if (!is_open()) {
                ec = not_open;
                return false;
            }
            ec = error_code();
            if (data_->is_alive) {
#ifndef BOOST_WINDOWS_API
                if (data_->ppid == ::getpid()) {
                    int ret = ::waitpid(data_->pid, &data_->status, WNOHANG);
                    if (ret == data_->pid) {
                        data_->is_alive = false;
                    } else if (ret == 0) {
                        data_->is_alive = true;
                    } else {
                        ec = last_system_error();
                        data_->is_alive = false;
                    }
                } else {
                    ProcessInfo info;
                    data_->is_alive = false;
                    while (!data_->handle->empty()) {
                        ProcessInfo & pi = data_->handle->front();
                        if (!get_process_info(info, pi.pid, "", ec)) {
                            LOG_S(Logger::kLevelDebug1, "is_alive (no such process) " << pi.pid);
                            pi = data_->handle->back();
                            data_->handle->pop_back();
                            continue;
                        }
                        if (!has_process_name(pi.bin_file, info.bin_file)) {
                            LOG_S(Logger::kLevelDebug1, "is_alive (not match process) " << pi.pid 
                                << " " << info.bin_file);
                            pi = data_->handle->back();
                            data_->handle->pop_back();
                            continue;
                        }
                        ProcessStat stat;
                        if ((ec = get_process_stat(pi.pid, stat))) {
                            LOG_S(Logger::kLevelDebug1, "is_alive (not process) " << pi.pid);
                            pi = data_->handle->back();
                            data_->handle->pop_back();
                            continue;
                        }
                        if (stat.state != ProcessStat::zombie && stat.state != ProcessStat::dead) {
                            data_->is_alive = true;
                        } else {
                            LOG_S(Logger::kLevelDebug1, "is_alive (process dead) " << pi.pid);
                        }
                        break;
                    }
                    if (!data_->is_alive) {
                        LOG_S(Logger::kLevelDebug1, "is_alive (process not exist) " << data_->pid);
                    }
                }
#else
//modify by zhaoyf 2013-11-29
//在app模式下，GetExitCodeProcess接口不支持，暂时屏蔽api
#if !STREAMSDK_WIN8_APP
            BOOL success = ::GetExitCodeProcess(data_->handle, &data_->status);
            assert(success);
            if (!success) {
                ec = last_system_error();
                return false;
            }
            data_->is_alive = data_->status == STILL_ACTIVE;
            if (!data_->is_alive) {
                ::CloseHandle(data_->handle);
                data_->handle = NULL;
            }
#else
            //assert(false);
            return false;
#endif
//modify end.
                
#endif
                return data_->is_alive;
            } else {
                return false;
            }
        }

        error_code Process::join(
            error_code & ec)
        {
            if (is_alive(ec)) {
#ifndef BOOST_WINDOWS_API
                if (::waitpid(data_->pid, &data_->status, 0) > 0) {
                    data_->is_alive = false;
                } else {
                    ec = last_system_error();
                }
#else
                if (WAIT_OBJECT_0 == ::WaitForSingleObject(data_->handle, INFINITE)) {
                    is_alive(ec);
                } else {
                    ec = last_system_error();
                }
#endif
            }
            return ec;
        }

#ifndef BOOST_WINDOWS_API
        static int write_fd = -1;
        static void sig_child(int signo)
        {
            char c = 0;
            ::write(write_fd, &c, 1);
        }
#endif

        error_code Process::timed_join(
            unsigned long milliseconds, 
            error_code & ec)
        {
            if (is_alive(ec)) {
#ifndef BOOST_WINDOWS_API
                if (data_->ppid == ::getpid()) {
                    if (::waitpid(data_->pid, &data_->status, WNOHANG) > 0) {
                        data_->is_alive = false;
                    } else {
                        int pipefd[2];
                        ::pipe(pipefd);
                        write_fd = pipefd[1];
                        struct sigaction sa;
                        struct sigaction sa_old;
                        sa.sa_handler = sig_child;
                        sa.sa_flags = SA_NODEFER;
                        sigaction(SIGCHLD, &sa, &sa_old);
                        struct pollfd fd;
                        fd.fd = pipefd[0];
                        fd.events = POLLIN;
                        while (true) {
                            int ret = ::waitpid(data_->pid, &data_->status, WNOHANG);
                            if (ret < 0) {
                                ec = last_system_error();
                                break;
                            } else if (ret > 0) {
                                ec.clear();
                                break;
                            }
                            ret = ::poll(&fd, 1, milliseconds);
                            if (ret < 0) {
                                if (errno != EINTR) {
                                    ec = last_system_error();
                                    break;
                                }
                            } else if (ret == 0) {
                                ec = error_code(ETIMEDOUT, boost::system::get_system_category());
                                break;
                            }
                        }
                        sigaction(SIGALRM, &sa_old, NULL);
                        ::close(pipefd[0]);
                        ::close(pipefd[1]);
                    }
                } else {
                    while (is_alive(ec) && milliseconds) {
                        boost::this_thread::sleep(
                            boost::posix_time::milliseconds(100));
                        milliseconds = milliseconds > 100 ? milliseconds - 100 : 0;
                    }
                    if (!ec && milliseconds == 0) {
                        ec = error_code(ETIMEDOUT, boost::system::get_system_category());
                    }
                }
#else
                DWORD waitResult = ::WaitForSingleObject(data_->handle, milliseconds);
                if (WAIT_OBJECT_0 == waitResult) {
                    is_alive(ec);
                } else {
                    ec = last_system_error();
                }
#endif
            }
            return ec;
        }

        error_code Process::signal(
            Signal sig, 
            error_code & ec)
        {
            if (is_alive(ec)) {
#ifndef BOOST_WINDOWS_API
                ::kill(data_->pid, sig.value());
                ec = last_system_error();
#else
#endif
            }
            return ec;
        }

        error_code Process::detach(
            error_code & ec)
        {
            if (!is_open()) {
                ec = not_open;
                return ec;
            }
#ifndef BOOST_WINDOWS_API
            if (data_->handle) {
                delete data_->handle;
            }
#else
            ::CloseHandle(data_->handle);
#endif
            delete data_;
            data_ = NULL;
            return ec = error_code();
        }

        error_code Process::kill(
            error_code & ec)
        {
            if (is_alive(ec)) {
#ifndef BOOST_WINDOWS_API
                if (::kill(data_->pid, SIGKILL) == 0) {
                    if (data_->ppid == ::getpid()) {
                        ::waitpid(data_->pid, &data_->status, 0);
                        ec = last_system_error();
                    }
                    if (data_->handle) {
                        delete data_->handle;
                    }
                } else {
                    ec = last_system_error();
                }
#else
//modify by zhaoyf 2013-11-29
//在app模式下，TerminateProcess接口不支持，暂时屏蔽api
#if !STREAMSDK_WIN8_APP
                assert(false);
                ::TerminateProcess(data_->handle, 99);
                ::CloseHandle(data_->handle);
#endif
//modify end.
#endif
                data_->handle = NULL;
                data_->status = 99;
                data_->is_alive = false;
                ec = last_system_error();
            }
            return ec;
        }

        int Process::exit_code(
            error_code & ec)
        {
            if (is_alive(ec)) {
                ec = still_alive;
                return -1;
            }
            ec = error_code();
#ifndef BOOST_WINDOWS_API
            if (WIFEXITED(data_->status)) {
                return WEXITSTATUS(data_->status);
            } else {
                return 0;
            }
#else
            return data_->status;
#endif
        }

        error_code Process::close(
            error_code & ec)
        {
            if (is_open()) {
                detach(ec);
            } else {
                ec = error_code();
            }
            if (data_) {
                delete data_;
                data_ = NULL;
            }
            return ec;
        }

        error_code Process::stat(
            ProcessStat & stat) const
        {
            error_code ec;
            if (data_ && data_->is_alive) {
#ifndef BOOST_WINDOWS_API
                ec = get_process_stat(data_->pid, stat);
#else
                ec = get_process_stat(data_->handle, stat);
#endif
            } else {
                ec = not_alive;
            }
            return ec;
        }

        error_code Process::statm(
            ProcessStatM & statm) const
        {
            error_code ec;
            if (data_ && data_->is_alive) {
#ifndef BOOST_WINDOWS_API
                ec = get_process_statm(data_->pid, statm);
#else
                ec = get_process_statm(data_->handle, statm);
#endif
            } else {
                ec = not_alive;
            }
            return ec;
        }

        int parent_id(
            int id)
        {
#ifndef BOOST_WINDOWS_API
            return (int)::getppid();
#else
            return 0;
#endif
        }

    } // namespace process

    namespace this_process
    {

        int id()
        {
#ifndef BOOST_WINDOWS_API
            return (int)::getpid();
#else
            return (int)::GetCurrentProcessId();
#endif
        }

        int parent_id()
        {
#ifndef BOOST_WINDOWS_API
            return (int)::getppid();
#else
            return process::parent_id(id());
#endif
        }

        bool notify_wait(
            error_code const & ec)
        {
#ifndef BOOST_WINDOWS_API
            char * value;
            if ((value = ::getenv("FRAMEWORK_PROCESS_WAIT_FILE"))) {
                int fd = parse<int>(value);
                ::unsetenv("FRAMEWORK_PROCESS_WAIT_FILE");
                char buffer[64];
                size_t len = strlen(ec.category().name());
                memcpy(buffer, ec.category().name(), len);
                buffer[len++] = ':';
                len += framework::process::detail::format(buffer + len, ec.value());
                ::write(fd, buffer, len);
                ::close(fd);
                return true;
            }
#else
//modify by zhaoyf 2013-11-29
//在app模式下，暂时无此接口需求，暂时屏蔽api
#if !STREAMSDK_WIN8_APP
            char Buffer[64];
            HANDLE hParent = INVALID_HANDLE_VALUE;
            if (::GetEnvironmentVariable(
                "FRAMEWORK_PROCESS_PARENT_ID", 
                Buffer, 
                sizeof(Buffer)) > 0) {
                    DWORD dwPId = (DWORD)framework::process::detail::parse(Buffer, strlen(Buffer));
                    hParent = ::OpenProcess(
                        PROCESS_ALL_ACCESS, 
                        FALSE, 
                        dwPId);
            }
            if (::GetEnvironmentVariable(
                "FRAMEWORK_PROCESS_WAIT_FILE", 
                Buffer, 
                sizeof(Buffer)) > 0) {
                    ::SetEnvironmentVariable(
                        "FRAMEWORK_PROCESS_WAIT_FILE", 
                        NULL);
                    HANDLE hFileWrite = (HANDLE)framework::process::detail::parse(Buffer, strlen(Buffer));
                    ::DuplicateHandle(
                        hParent, 
                        hFileWrite, 
                        GetCurrentProcess(), 
                        &hFileWrite, 
                        0, 
                        FALSE, 
                        DUPLICATE_SAME_ACCESS);
                    size_t len = strlen(ec.category().name());
                    memcpy(Buffer, ec.category().name(), len);
                    Buffer[len++] = ':';
                    len += framework::process::detail::format(Buffer + len, ec.value());
                    DWORD NumberOfBytesWrite = 0;
                    ::WriteFile(
                        hFileWrite, 
                        Buffer, 
                        len, 
                        &NumberOfBytesWrite, 
                        NULL);
                    ::CloseHandle(hFileWrite);
                    return true;
            }
#else
            assert(false);
#endif
            
//modify end.	
#endif
            return false;
        }

    } // namespace this_process

} // namespace framework
