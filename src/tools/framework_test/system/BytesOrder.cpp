// BytesOrder.cpp

#include "tools/framework_test/Common.h"

#include <framework/system/BytesOrder.h>
using namespace framework::configure;
using namespace framework::system;

#pragma pack(push, 1)

struct PackedStruct
{
    boost::uint8_t uint8_;
    boost::uint32_t uint32_;
    boost::uint8_t uint8_3[3];
};

#pragma pack(pop)

struct NormalStruct
{
    boost::uint8_t uint8_;
    boost::uint32_t uint32_;
};

static void do_with_struct(
                           NormalStruct & obj, 
                           NormalStruct & obj1)
{
    std::cout << "read " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
    std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
    std::cout << "assign " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
    obj.uint8_ = 8;
    obj.uint32_ = 32;
    std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
    std::cout << "copy " << (boost::uint32_t *)&obj1.uint8_ << " " << &obj1.uint32_ << std::endl;
    obj1 = obj;
    std::cout << (int)obj1.uint8_ << " " << obj1.uint32_ << std::endl;
}

static void do_with_struct(
                           PackedStruct & obj,
                           PackedStruct & obj1)
{
    std::cout << "read " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
    std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
    std::cout << "assign " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
    obj.uint8_ = 8;
    obj.uint32_ = 32;
    std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
    std::cout << "copy " << (boost::uint32_t *)&obj1.uint8_ << " " << &obj1.uint32_ << std::endl;
    obj1 = obj;
    std::cout << (int)obj1.uint8_ << " " << obj1.uint32_ << std::endl;
}

void do_with(boost::uint32_t n)
{
}

void test_bytes_alignment()
{
    try {
        std::cout << "NormalStruct" << std::endl;
        NormalStruct obj;
        NormalStruct obj1;
        std::cout << "read " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
        std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
        std::cout << "assign " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
        obj.uint8_ = 8;
        obj.uint32_ = 32;
        std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
        std::cout << "copy " << (boost::uint32_t *)&obj1.uint8_ << " " << &obj1.uint32_ << std::endl;
        obj1 = obj;
        std::cout << (int)obj1.uint8_ << " " << obj1.uint32_ << std::endl;
        std::cout << "do_with_refrence" << std::endl;
        do_with_struct(obj, obj1);
    } catch (std::exception const & err) {
        std::cout << err.what() << std::endl;
    }

    try {
        std::cout << "PackedStruct" << std::endl;
        PackedStruct obj;
        PackedStruct obj1;
        std::cout << "read " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
        std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
        std::cout << "assign " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
        obj.uint8_ = 8;
        obj.uint32_ = 32;
        std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
        std::cout << "copy " << (boost::uint32_t *)&obj1.uint8_ << " " << &obj1.uint32_ << std::endl;
        obj1 = obj;
        std::cout << (int)obj1.uint8_ << " " << obj1.uint32_ << std::endl;
        std::cout << "do_with_refrence" << std::endl;
        do_with_struct(obj, obj1);
    } catch (std::exception const & err) {
        std::cout << err.what() << std::endl;
    }

    try {
        std::cout << "PackedStruct (raw pointer)" << std::endl;
        int n = 3;
        do_with(n);
        char buf[64];
        char buf1[64];
        PackedStruct & obj = *(PackedStruct *)(buf + n);
        PackedStruct & obj1 = *(PackedStruct *)(buf1 + n);
        std::cout << "read " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
        std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
        std::cout << "assign " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
        obj.uint8_ = 8;
        obj.uint32_ = 32;
        std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
        std::cout << "copy " << (boost::uint32_t *)&obj1.uint8_ << " " << &obj1.uint32_ << std::endl;
        obj1 = obj;
        std::cout << (int)obj1.uint8_ << " " << obj1.uint32_ << std::endl;
        std::cout << "do_with_refrence" << std::endl;
        do_with_struct(obj, obj1);
    } catch (std::exception const & err) {
        std::cout << err.what() << std::endl;
    }

    try {
        std::cout << "NormalStruct (raw pointer)" << std::endl;
        int n = 3;
        do_with(n);
        char buf[64];
        char buf1[64];
        NormalStruct & obj = *(NormalStruct *)(buf + n);
        NormalStruct & obj1 = *(NormalStruct *)(buf1 + n);
        std::cout << "read " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
        std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
        std::cout << "assign " << (boost::uint32_t *)&obj.uint8_ << " " << &obj.uint32_ << std::endl;
        obj.uint8_ = 8;
        obj.uint32_ = 32;
        std::cout << (int)obj.uint8_ << " " << obj.uint32_ << std::endl;
        std::cout << "copy " << (boost::uint32_t *)&obj1.uint8_ << " " << &obj1.uint32_ << std::endl;
        obj1 = obj;
        std::cout << (int)obj1.uint8_ << " " << obj1.uint32_ << std::endl;
        std::cout << "do_with_refrence" << std::endl;
        do_with_struct(obj, obj1);
    } catch (std::exception const & err) {
        std::cout << err.what() << std::endl;
    }
}

static void test_bytes_order(Config & conf)
{
    unsigned long v = 1234;
    if (BytesOrder::host_to_net_long(v) == v) {
        std::cout << "Big Endian" << std::endl;
    } else {
        std::cout << "Little Endian" << std::endl;
    }

    short s = 0x1122;
    short s_l = BytesOrder::host_to_little_endian(s);
    short s_b = BytesOrder::host_to_big_endian(s);

    std::cout << "s_l=" << s_l << " s_b=" << s_b << std::endl;

    int i = 0x11223344;
    int i_l = BytesOrder::host_to_little_endian(i);
    int i_b = BytesOrder::host_to_big_endian(i);

    std::cout << "i_l=" << i_l << " i_b=" << i_b << std::endl;

    long l = 0x11223344;
    long l_l = BytesOrder::host_to_little_endian(l);
    long l_b = BytesOrder::host_to_big_endian(l);

    std::cout << "l_l=" << l_l << " l_b=" << l_b << std::endl;

    unsigned long long ll = 0x1122334455667788ULL;
    unsigned long long ll_l = BytesOrder::host_to_little_endian(ll);
    unsigned long long ll_b = BytesOrder::host_to_big_endian(ll);

    std::cout << "ll_l=" << ll_l << " ll_b=" << ll_b << std::endl;

    double d = 10.0;
    double d_l = BytesOrder::host_to_little_endian(d);
    double d_b = BytesOrder::host_to_big_endian(d);

    std::cout << "d_l=" << d_l << " d_b=" << d_b << std::endl;
    char * d_b_c = (char *)&d_b;
    std::cout << "d_b_c=";
    for (size_t i = 0; i < sizeof(d_b); ++i)
        std::cout << (int)d_b_c[i] << ", ";
    std::cout << std::endl;
}

static TestRegister test("bytes_order", test_bytes_order);
