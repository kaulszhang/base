//rtmp push

#include <rtmp/rtmp_sys.h>
#include <rtmp/rtmp.h>

#include <iostream>
#include <fstream>

#ifdef _DEBUG
FILE *netstackdump = 0;
FILE *netstackdump_read = 0;
#endif

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << "cmd line error" << std::endl;
        return 0;
    }
#ifdef _DEBUG
    netstackdump = fopen("netstackdump", "wb");
    netstackdump_read = fopen("netstackdump_read", "wb");
#endif

    std::ifstream is;
    is.open("/tmp/1.flv", std::ios::binary);
    if (!is.is_open()) {
        std::cout << "open file failed" << std::endl;
        return 0;
    }

    RTMP rtmp = {0};
    RTMP_Init(&rtmp);
    //if (!RTMP_SetupURL(&rtmp, argv[1])) {
    //    std::cout << "set url failed" << std::endl;
    //    return 0;
    //}
    std::cout << "rtmp url: " << argv[1] << std::endl;
    if (!RTMP_SetupURL(&rtmp, argv[1])) {
        std::cout << "set url failed" << std::endl;
        return 0;
    }

    // push to server;
    RTMP_EnableWrite(&rtmp);
    if (!RTMP_Connect(&rtmp, NULL)) {
        std::cout << "rtmp connect failed" << std::endl;
        return 0;
    }

    if (!RTMP_ConnectStream(&rtmp, 0)) {
        std::cout << "rtmp connect stream failed" << std::endl;
        return 0;
    }

    char buffer[1024];
    int act_val = 0;
    while(true) {
        act_val = is.readsome(buffer, 1024);
        RTMP_Write(&rtmp, buffer, act_val);
        if (act_val < 1024) {
            break;
        }
        usleep(10000);
    }
    std::cout << "rtmp push finish" << std::endl;
    is.close();
    RTMP_Close(&rtmp);
    return 0;

}
