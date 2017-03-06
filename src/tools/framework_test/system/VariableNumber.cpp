// VariableNumber.cpp

#include "tools/framework_test/Common.h"

#include <framework/system/VariableNumber.h>
using namespace framework::configure;
using namespace framework::system;

template <typename _Ty>
struct Define;

template <>
struct Define<boost::uint16_t>
{
    static const boost::uint16_t min = 0;
    static const boost::uint16_t max = 0x3fff;
};

template <>
struct Define<boost::uint32_t>
{
    static const boost::uint32_t min = 0;
    static const boost::uint32_t max = 0x0fffffff;
};

template <>
struct Define<boost::uint64_t>
{
    static const boost::uint64_t min = 0;
    static const boost::uint64_t max = 0x00ffffffffffffffULL;
};

template <>
struct Define<boost::int16_t>
{
    static const boost::int16_t min = 0xE001;
    static const boost::int16_t max = 0x2000;
};

template <>
struct Define<boost::int32_t>
{
    static const boost::int32_t min = 0xF8000001;
    static const boost::int32_t max = 0x08000000;
};

template <>
struct Define<boost::int64_t>
{
    static const boost::int64_t min = 0xFF80000000000001ULL;
    static const boost::int64_t max = 0x0080000000000000ULL;
};

template <typename _Ty>
void test_variable_number_bed()
{
    for (_Ty i = Define<_Ty>::min; i < Define<_Ty>::max; ++i) {
        VariableNumber<_Ty> n(i);
        assert(i == n.value());
        std::cout << n.size() << "\t" << n.value() << std::endl;
    }
}

static void test_variable_number(Config & conf)
{
//     test_variable_number_bed<boost::uint16_t>();
//     test_variable_number_bed<boost::uint32_t>();
//     test_variable_number_bed<boost::uint64_t>();
     test_variable_number_bed<boost::int16_t>();
//     test_variable_number_bed<boost::int32_t>();
//     test_variable_number_bed<boost::int64_t>();
}

static TestRegister test("variable_number", test_variable_number);
