// ParseStl.h

#ifndef _FRAMEWORK_STRING_PARSE_STL_H_
#define _FRAMEWORK_STRING_PARSE_STL_H_

#include "framework/string/Parse.h"
#include "framework/string/Slice.h"
#include "framework/string/StringToken.h"

#include <iterator>

namespace framework
{
    namespace string
    {

#ifndef _CONTAINER_DEF_
#define _CONTAINER_DEF_
        template<
            typename _Ct
        >
        struct container_def
        {
            static char const * prefix() { return "{"; };
            static char const * delim() { return ","; };
            static char const * suffix() { return "}"; };
        };
#endif // _CONTAINER_DEF_

        // 解析集合类型的数据
        struct parse_container
        {
            template<
                typename _Ct
            >
            static boost::system::error_code invoke(
            std::string const & str, 
            _Ct & ct)
            {
                char const * prefix = container_def<_Ct>::prefix();
                char const * delim = container_def<_Ct>::delim();
                char const * suffix = container_def<_Ct>::suffix();
                ct.clear();
                typedef typename _Ct::value_type value_type;
                return slice<value_type>(str, std::inserter(ct, ct.end()), delim, prefix, suffix);
            }
        };

#ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

        template<typename _Container>
        struct container_def<std::insert_iterator<_Container> >
            : container_def<typename _Container::iterator>
        {
        };

        template<typename _Kty, typename _Alloc>
        struct Parser2<std::vector<_Kty, _Alloc> >
            : std::binary_function<std::string, std::vector<_Kty, _Alloc>, boost::system::error_code>
        {
            boost::system::error_code operator () (
                std::string const & str, 
                std::vector<_Kty, _Alloc> & v)
            {
                typedef typename std::vector<_Kty, _Alloc>::value_type value_type;
                return parse_container::invoke(str, v);
            }
        };

        template<typename _Kty, typename _Alloc>
        struct Parser2<std::list<_Kty, _Alloc> >
            : std::binary_function<std::string, std::list<_Kty, _Alloc>, boost::system::error_code>
        {
            boost::system::error_code operator () (
                std::string const & str, 
                std::list<_Kty, _Alloc> & v)
            {
                typedef typename std::list<_Kty, _Alloc>::value_type value_type;
                return parse_container::invoke(str, v);
            }
        };

        template<typename _Kty, typename _Pr, typename _Alloc>
        struct Parser2<std::set<_Kty, _Pr, _Alloc> >
            : std::binary_function<std::string, std::set<_Kty, _Pr, _Alloc>, boost::system::error_code>
        {
            boost::system::error_code operator () (
                std::string const & str, 
                std::set<_Kty, _Pr, _Alloc> & v)
            {
                typedef typename std::set<_Kty, _Pr, _Alloc>::value_type value_type;
                return parse_container::invoke(str, v);
            }
        };

        template<typename _Kty, typename _Ty, typename _Pr, typename _Alloc>
        struct Parser2<std::map<_Kty, _Ty, _Pr, _Alloc> >
            : std::binary_function<std::string, std::map<_Kty, _Ty, _Pr, _Alloc>, boost::system::error_code>
        {
            boost::system::error_code operator () (
                std::string const & str, 
                std::map<_Kty, _Ty, _Pr, _Alloc> & v)
            {
                typedef typename std::map<_Kty, _Ty, _Pr, _Alloc>::value_type value_type;
                return parse_container::invoke(str, v);
            }
        };

#ifndef _PAIR_DELIM_
#define _PAIR_DELIM_
        template<typename _Ty1, typename _Ty2>
        struct pair_def
        {
            static char const * delim() { return ":"; };
        };
#endif

        template<typename _Ty1, typename _Ty2>
        struct Parser2<std::pair<_Ty1, _Ty2> >
            : std::binary_function<std::string, std::pair<_Ty1, _Ty2>, boost::system::error_code>
        {
            boost::system::error_code operator () (
                std::string const & str, 
                std::pair<_Ty1, _Ty2> & v)
            {
                StringToken st(str, pair_def<_Ty1, _Ty2>::delim());
                std::string str1;
                typedef typename boost::remove_const<_Ty1>::type typef;
                boost::system::error_code ec;
                return (st.next_token(str1, ec)
                    || parse2(str1, const_cast<typef &>(v.first))
                    || parse2(st.remain(), v.second)) ? 
                    framework::system::logic_error::invalid_argument
                    : framework::system::logic_error::succeed;
            }
        };

#elif !defined BOOST_NO_FUNCTION_TEMPLATE_ORDERING

        template<typename _Kty, typename _Alloc>
        boost::system::error_code from_string(
            std::string const & str, 
            std::vector<_Kty, _Alloc> & v)
        {
            typedef typename std::vector<_Kty, _Alloc>::value_type value_type;
            return parse_container::invoke(str, v);
        }

        template<typename _Kty, typename _Alloc>
        boost::system::error_code from_string(
            std::string const & str, 
            std::list<_Kty, _Alloc> & v)
        {
            typedef typename std::list<_Kty, _Alloc>::value_type value_type;
            return parse_container::invoke(str, v);
        }

        template<typename _Kty, typename _Pr, typename _Alloc>
        boost::system::error_code from_string(
            std::string const & str, 
            std::set<_Kty, _Pr, _Alloc> & v)
        {
            typedef typename std::set<_Kty, _Pr, _Alloc>::value_type value_type;
            return parse_container::invoke(str, v);
        }

        template<typename _Kty, typename _Ty, typename _Pr, typename _Alloc>
        boost::system::error_code from_string(
            std::string const & str, 
            std::map<_Kty, _Ty, _Pr, _Alloc> & v)
        {
            typedef typename std::map<_Kty, _Ty, _Pr, _Alloc>::value_type value_type;
            return parse_container::invoke(str, v);
        }

        template<typename _Ty1, typename _Ty2>
        boost::system::error_code from_string(
            std::string const & str, 
            std::pair<_Ty1, _Ty2> & v)
        {
            StringToken st(str.substr(1, str.size() - 2), ",");
            std::string str1;
            std::string str2;
            typedef typename boost::remove_const<_Ty1>::type typef;
            boost::system::error_code ec;
            return (st.next_token(str1, ec)
                || parse2(str1, const_cast<typef &>(v.first))
                || parse2(st.remain(), v.second)) ? 
                framework::system::logic_error::invalid_argument
                : framework::system::logic_error::succeed;
        }

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

    } // namespace string
} // namespace framework

#ifndef FRAMEWORK_STRING_CONTAINER_DEFINE
#define FRAMEWORK_STRING_CONTAINER_DEFINE(_container, _prefix, _delim, _suffix) \
    namespace framework { namespace string { \
    template<> \
struct container_def<_container::iterator> \
{ \
    static char const * prefix() { return _prefix; }; \
    static char const * delim() { return _delim; }; \
    static char const * suffix() { return _suffix; }; \
}; \
    template<>  \
struct container_def<_container::const_iterator> \
{ \
    static char const * prefix() { return _prefix; }; \
    static char const * delim() { return _delim; }; \
    static char const * suffix() { return _suffix; }; \
}; \
}}
#endif // CONTAINER_DEFINE

#ifndef FRAMEWORK_STRING_PAIR_DEFINE
#define FRAMEWORK_STRING_PAIR_DEFINE(_Ty1, _Ty2, _delim) \
    namespace framework { namespace string { \
    template<> \
struct pair_def<_Ty1, _Ty2> \
{ \
    static char const * delim() { return _delim; }; \
}; \
}}
#endif // PAIR_DEFINE

#endif // _FRAMEWORK_STRING_PARSE_STL_H_
