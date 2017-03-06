// test_serialize.cpp

#include "tools/util_test/Common.h"

#include <framework/system/LogicError.h>

#include <util/serialization/stl/vector.h>
#include <util/serialization/stl/deque.h>
#include <util/serialization/stl/map.h>
#include <util/serialization/ErrorCode.h>
#include <util/archive/TextIArchive.h>
#include <util/archive/TextOArchive.h>
#include <util/archive/ConsoleOArchive.h>
#include <util/archive/ArchiveBuffer.h>
using namespace util::archive;

using namespace framework::configure;

#include <istream>
#include <ostream>

struct StructEnum
{
    enum Enum
    {
        a, 
        b, 
        c, 
    };
};

namespace object
{
    template <typename T>
    struct Object
    {
        T i;
    };

    template <typename Archive, typename T>
    void serialize(Archive & ar, object::Object<T> & o)
    {
        ar & o.i;
    }
}

namespace util
{
    namespace serialization
    {
        //template <typename Archive, typename T>
        //void serialize(Archive & ar, object::Object<T> & o)
        //{
        //    ar & o.i;
        //}
    }
}

void test_serialize(Config & conf)
{
    std::string str = "ssss";
    int a = 4;
    int b = 6;
    boost::system::error_code ec = framework::system::logic_error::io_busy;
    std::vector<std::string> vec;
    vec.push_back("a");
    vec.push_back("b");
    vec.push_back("c");

    std::deque<std::string> deq;
    deq.push_back("a");
    deq.push_back("ab");
    deq.push_back("abc");

    std::map<std::string, std::string> m;
    typedef std::pair<std::string, std::string> str_pair;
    str_pair aa = str_pair("a", "a_map");
    str_pair bb = str_pair("b", "b_map");
    str_pair cc = str_pair("c", "c_map");
    m.insert(aa);
    m.insert(bb);
    m.insert(cc);

    object::Object<int> obj;
    obj.i = 1;

    StructEnum::Enum e = StructEnum::b;

    char cbuf[1024];
    ArchiveBuffer<> buf(cbuf, sizeof(cbuf));

    std::ostream os(&buf);
    TextOArchive<> oa(os);

    oa << a << ec << vec << deq << m << str << b << e << obj;

    std::istream is(&buf);
    TextIArchive<> ia(is);

    ia >> a >> ec >> vec >> deq >> m >> str >> b >> e >> obj;

    {
        ConsoleOArchive oa;
        oa << (SERIALIZATION_NVP(a));
        oa << (SERIALIZATION_NVP(ec));
        oa << SERIALIZATION_NVP(vec) 
            << SERIALIZATION_NVP(deq) 
            << SERIALIZATION_NVP(m) 
            << SERIALIZATION_NVP(str);
        oa << (SERIALIZATION_NVP(b));
        oa << (SERIALIZATION_NVP(e));
        oa << SERIALIZATION_NVP(obj);
    }
}

static TestRegister test("serialize", test_serialize);
