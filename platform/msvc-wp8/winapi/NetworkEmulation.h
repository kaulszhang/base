// NetworkEmulation.h

#pragma once

namespace NetworkEmulation
{

    struct adapter {
        int        len;
        int        lan_type;
        GUID    id;
        //char    addr[];
    };

	int WINAPI_DECL getadapters(
        _Out_  char *adapters,
        _In_   int len
        );

}
