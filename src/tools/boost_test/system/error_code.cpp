// error_code.cpp

#include "tools/boost_test/Common.h"

using namespace framework::configure;

using namespace boost::system;

namespace test_error
{
    enum errors
    {
        test_error1 = 16, 
        test_error2, 
    };

    class TestErrorCategory : 
        public error_category
    {
    public:
        const char * name() const BOOST_SYSTEM_NOEXCEPT
        {
            return "test";
        }

        std::string message( int ev ) const
        {
            switch (ev) {
            case test_error1:
                return "test_error1";
                break;
            case test_error2:
                return "test_error2";
                break;
            default:
                assert(0);
                return "";
            }
        }
    };

    
    inline const boost::system::error_category & get_category()
    {
        static TestErrorCategory instance;
        return instance;
    }

    inline boost::system::error_code make_error_code(
        errors e)
    {
        return boost::system::error_code(
            static_cast<int>(e), get_category());
    }

    static boost::system::error_category const & category = get_category();

}

namespace boost
{
    namespace system
    {
        template <>
        struct is_error_code_enum<test_error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

        using test_error::make_error_code;

    }
}

static void test_error_code(Config & conf)
{
    error_code a1(test_error::test_error1);
    error_code a2(a1);
    a2 = test_error::test_error2;
    a1 = a2;
}

static TestRegister test("error_code", test_error_code);
