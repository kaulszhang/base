// Url.cpp

#include "tools/framework_test/Common.h"

#include <framework/string/Url.h>
using namespace framework::string;
using namespace framework::configure;

static void test_url(Config & conf)
{
    {
    Url url("http://www.google.cn/search?hl=zh-CN&newwindow=1&rlz=1T4GGLL_zh-CNCN330CN330&q=1gsdg");
    assert(url.protocol() == "http");
    assert(url.user() == "");
    assert(url.password() == "");
    assert(url.host() == "www.google.cn");
    assert(url.svc() == "");
    assert(url.path() == "/search");
    assert(url.param("hl") == "zh-CN");
    assert(url.param("newwindow") == "1");
    assert(url.param("rlz") == "1T4GGLL_zh-CNCN330CN330");
    assert(url.param("q") == "1gsdg");
    }

    {
    Url url("http://[fe80::9862:baf0:b101:507a%11]/search?hl=zh-CN&newwindow=1&rlz=1T4GGLL_zh-CNCN330CN330&q=1gsdg");
    assert(url.protocol() == "http");
    assert(url.user() == "");
    assert(url.password() == "");
    assert(url.host() == "[fe80::9862:baf0:b101:507a%11]");
    assert(url.svc() == "");
    assert(url.path() == "/search");
    assert(url.param("hl") == "zh-CN");
    assert(url.param("newwindow") == "1");
    assert(url.param("rlz") == "1T4GGLL_zh-CNCN330CN330");
    assert(url.param("q") == "1gsdg");
    }

    {
    Url url("file:///1.ts?dd=ff");
    assert(url.protocol() == "file");
    assert(url.user() == "");
    assert(url.password() == "");
    assert(url.host() == "");
    assert(url.svc() == "");
    assert(url.path() == "/1.ts");
    assert(url.param("dd") == "ff");
    }

    {
    Url url("ppvod://1d7c1WCcaMih2c7j4a5aeaaYrK2Up2Nae6WYnaGUp2pae6qYrqCUqmdaaZ2h2Nmj?dd=ff");
    assert(url.protocol() == "ppvod");
    assert(url.user() == "");
    assert(url.password() == "");
    assert(url.host() == "");
    assert(url.svc() == "");
    assert(url.path() == "/1d7c1WCcaMih2c7j4a5aeaaYrK2Up2Nae6WYnaGUp2pae6qYrqCUqmdaaZ2h2Nmj");
    assert(url.param("dd") == "ff");
    }

    {
    Url url("pplive2://e02603640db84df39b7190b38b5ee1c2-5-382?dd=ff");
    assert(url.protocol() == "pplive2");
    assert(url.user() == "");
    assert(url.password() == "");
    assert(url.host() == "");
    assert(url.svc() == "");
    assert(url.path() == "/e02603640db84df39b7190b38b5ee1c2-5-382");
    assert(url.param("dd") == "ff");
    }

    {
    Url url("/e02603640db84df39b7190b38b5ee1c2-5-382?dd=ff");
    assert(url.protocol() == "");
    assert(url.user() == "");
    assert(url.password() == "");
    assert(url.host() == "");
    assert(url.svc() == "");
    assert(url.path() == "/e02603640db84df39b7190b38b5ee1c2-5-382");
    assert(url.param("dd") == "ff");
    }
}

static TestRegister test("url", test_url);
