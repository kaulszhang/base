// NetworkEmulation.cpp

#include <Windows.h>

#define WINAPI_DECL     __declspec(dllexport)

#include "NetworkEmulation.h"

namespace NetworkEmulation
{

    int WINAPI_DECL getadapters(
        _Out_  char *adapters,
        _In_   int len
        )
    {
        char *buf = adapters;
        Windows::Foundation::Collections::IVectorView<Windows::Networking::HostName ^> ^ vec = 
            Windows::Networking::Connectivity::NetworkInformation::GetHostNames();
        Platform::String ^ svc = ref new Platform::String(L"0");
        for (unsigned int i = 0; i < vec->Size; i++) {
            Windows::Networking::HostName ^ host = vec->GetAt(i);
            int addrlen = host->RawName->Length() + 1;
            if ((int)sizeof(adapter) + addrlen <= len) {
                adapter * a = (adapter *)buf;
                a->len = sizeof(adapter) + addrlen;
                if (host->IPInformation && host->IPInformation->NetworkAdapter) {
                    a->lan_type = host->IPInformation->NetworkAdapter->IanaInterfaceType;
                    //a->id = host->IPInformation->NetworkAdapter->NetworkAdapterId;
                }
                char * addr = (char *)(a + 1);
                WideCharToMultiByte(CP_ACP, 0, host->RawName->Data(), -1, addr, addrlen, NULL, FALSE);
                buf += a->len;
                len -= a->len;
            }
        }
        return buf - adapters;
    }

}
