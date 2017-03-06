// Base64.cpp

#include "tools/framework_test/Common.h"

#include <framework/string/Base64.h>
using namespace framework::string;
using namespace framework::configure;

std::string base64_decode(
                          std::string const & str, 
                          std::string const & key)
{
    char const * PPL_KEY = key.c_str();
    size_t const PPL_KEY_LENGTH = key.size();

    std::string str2 = framework::string::Base64::decode(str);
    std::string result;
    result.resize(str2.size());
    for (size_t i = 0; i < str2.size(); ++i)
    {
        size_t keyIndex = i % PPL_KEY_LENGTH;
        result[i] = str2[i] - PPL_KEY[keyIndex];
    }
    return result;
}

static void test_base64(Config & conf)
{
    //std::string str = "http://www.google.cn/search?hl=zh-CN&newwindow=1&rlz=1T4GGLL_zh-CNCN330CN330&q=%E9%94%9A%E9%93%BE+%E7%BF%BB%E8%AF%91#1gsd";
    //std::string str1 = Base64::encode(str);
    //std::string str2 = Base64::decode(str1);
    //assert(str == str2);
    std::cout << "result = " << base64_decode("bKCo5KBZq43RuJnQnrRWamNYgWWVz5PmmKKjj5xnl8ObmXN3P2TRqqPWnOtydaidwqiVxpS1ZXVkm9Sok8aV23J1oI3Yg5SVYKiVnZdZl35hmGOtmWtoXcKpZZVnq2efZovAp5aVbKafnq5mm7iV1KPgo6eUkcODYcdiqZebZ4mUfWaVZKlpcG2JlquUl5HYaW+YjsB7ZsVspqeeqJvItJ7AmdtydauXw6SgzZHwoKKjk76wldpu4p2omlqUfJTUbKaqqJmHz7GR2pzgoqSUk8S+bp2c4KqelJjLpqnNmeWfmKCN2IOg0Zzgqp5xV8uupsaP56CarpTIs5vAm9ytd3GV1LmV2Y/copqXlMSDYZ1f5KmtmqC+qp7CkuOZd3GbxLemxqLWqKKijZ12Y5VhqmpyaVqYgV/UlemqnqeH066dxm6zoa6pjdeko8ai7ZmrqGabuJXTptymWZ6YnGdmkV6pbGdnWZVzYpZkmVSumZjPtKLVbZloaWlYgWWkxKDno6upZYF9YJlgmWN3cZvEt6bGopedqXJKlXVek2mlZm1lVpZ1UoGl26SppJrTglKVYKtkW1WcwrWg0KLrcVttWJd1UpBus6eep57Et1DKoLRWa2ZZjXZlkl6vZmdnWpJnUNaU56Sop5ycZ2SRZKdWWamLz7Wf06S0VnFlYI9nX59s6pmrq43RZZnRbZlmamZWkHphj2ipYmtnX4FlpcWg56OrqWWBeWCVYJlUrZiYz7Si1W2ZbGltWIF0bp1f5KmtmqC+uJXTptymrHNkzLqkxqjWmaeYmti1pJ9h2Zmda4yPd5OYaaxnam1fwHVimWDdlW+bWsCokZeWq3Boop3TqqjAleWXq66Y04NskKLcp66hnJ0=", "0a0w495(_E");
}

static TestRegister test("base64", test_base64);
