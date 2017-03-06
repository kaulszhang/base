// MemoryInfo.cpp

#include "framework/Framework.h"
#include "framework/memory/MemoryInfo.h"


#ifdef BOOST_WINDOWS_API
#include <windows.h>
#elif (defined __MACH__)
#include <mach/mach.h>
#include <mach/mach_host.h>
#elif (defined __ANDROID__)
#include "framework/string/Slice.h"
#elif (defined __linux__)
#include <sys/sysinfo.h>
#else

#endif

namespace framework
{

    namespace memory
    {
#ifdef __ANDROID__
        static const boost::uint32_t LINE_BUF_SIZE=128;
        static const char* MEMORY_FILE = "/proc/meminfo";
        static const char* FREE_MEMORY_KEY = "MemFree:";
        static void parse(const char* line, boost::uint64_t& free_size)
        {
            std::string str_line(line);
            str_line.erase(0,str_line.find_first_not_of(" "));
            std::vector<std::string> params;
            framework::string::slice<std::string>(str_line, std::inserter(params, params.end()), " ");
            if(params.size() > 0)
            {
                framework::string::parse2(params[0].c_str(), free_size);
                free_size *= 1024;
            }
            else
                assert(0);
        }

        static void read_android_meminfo(boost::uint64_t& free_size)
        {
            free_size = 0;

            char line[LINE_BUF_SIZE];
            FILE *fp = NULL;
            fp = fopen(MEMORY_FILE, "r");
            if(NULL == fp) {
                return;
            }

            char *rs = NULL;
            while(fgets(line, sizeof(line), fp))
            {
                if(rs = strstr(line, FREE_MEMORY_KEY))
                {
                    parse(line+strlen(FREE_MEMORY_KEY),free_size);
                    break;
                }
            }
            fclose(fp);
        }
#endif

        boost::uint64_t MemoryInfo::free_memory_size()
        {
            boost::uint64_t free_size = 0;
#ifdef BOOST_WINDOWS_API 
#if defined(WINRT)||defined(WIN_PHONE)
            //暂时废弃此api，返回0表示api调用失败，后续需求需要再寻找win8api实现
            assert(false);
            return 0;
#else
            MEMORYSTATUS memstatus;
            memset(&memstatus,0,sizeof(memstatus));
            memstatus.dwLength =sizeof(memstatus);
            GlobalMemoryStatus(&memstatus);
            free_size = memstatus.dwAvailPhys ;
#endif
#elif (defined __MACH__)
            mach_port_t host_port = mach_host_self();
            mach_msg_type_number_t host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);

            vm_size_t pagesize;
            host_page_size(host_port, &pagesize);

            vm_statistics_data_t vm_stat;
            if(host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) == KERN_SUCCESS)
            {
                free_size = vm_stat.free_count * pagesize;
            }
#elif (defined __ANDROID__)
            read_android_meminfo(free_size);
#elif (defined __linux__)
            struct sysinfo s_info;
            memset(&s_info,0,sizeof(s_info));
            int ret = ::sysinfo(&s_info);
            if(0 == ret)
            {
                free_size = s_info.freeram;
            }
#else
#endif
            return free_size;
        }
    }
}
