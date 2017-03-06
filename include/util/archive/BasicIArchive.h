// BasicIArchive.h

#ifndef _UTIL_ARCHIVE_BASIC_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_BASIC_I_ARCHIVE_H_

#include "util/archive/BasicArchive.h"

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_enum.hpp>

namespace util
{
    namespace archive
    {

        struct LoadAccess
        {
            template <typename Archive>
            static void sub_start(Archive & ar)
            {
                ar.sub_start();
            }

            template <typename Archive>
            static void sub_end(Archive & ar)
            {
                ar.sub_end();
            }

            template <typename Archive, typename T>
            static void load_wrapper(Archive & ar, T & t)
            {
                ar.load_wrapper(t);
            }
        };

        /** 
            �����ĵ��Ļ���

            �ṩ��>>������&������������
            ����Դ������������׼���͵Ķ�
            @param Archive ������basic_iarchive�������ĵ��࣬ʵ�ֶԻ������͵�����
         */
        template <typename Archive>
        class BasicIArchive
            : public BasicArchive<Archive>
        {
        public:
            typedef boost::mpl::false_ is_saving; ///< �����ù鵵�����ǡ����ڱ������ݵ�
            typedef boost::mpl::true_ is_loading; ///< �����ù鵵�����ڼ������ݵ�
            friend struct LoadAccess;

        public:
            /// ����ָ�����͵Ķ�
            struct load_pointer
            {
                template<class T>
                static void invoke(
                    Archive & ar, 
                    T & t)
                {
                   ar.operator >> (*t);
                }
            };

            /// �����������л�
            struct load_array
            {
                template<class T>
                static void invoke(
                    Archive & ar, 
                    T const & t)
                {
                    size_t current_count = sizeof(t) / (
                        static_cast<const char *>(static_cast<const void *>(&t[1])) 
                        - static_cast<const char *>(static_cast<const void *>(&t[0]))
                        );
                    size_t count;
                    ar.operator >> (SERIALIZATION_NVP(count));
                    if (count > current_count) {
                        ar.fail();
                        return;
                    }
                    ar.operator >> (make_array(static_cast<T *>(&t[0]), count));
                }
            };

            /// ����ö�����͵Ķ�
            struct load_enum
            {
                template<class T>
                static void invoke(
                    Archive & ar, 
                    T & t)
                {
                    int i;
                    ar.load(i);
                    t = static_cast<T>(i);
                }
            };

            struct load_non_pointer
            {

                /// �����װ���͵Ķ�
                struct load_wrapper
                {
                    template<class T>
                    static void invoke(
                        Archive & ar, 
                        T & t)
                    {
                        // ֱ�ӵ���Archive��load������ͨ��load_access��ת
                        LoadAccess::load_wrapper(ar, t);
                    }
                };

                /// ����������͵Ķ�
                struct load_primitive
                {
                    template<class T>
                    static void invoke(
                        Archive & ar, 
                        T & t)
                    {
                        // ֱ�ӵ���Archive��load������ͨ��load_access��ת
                       ar.load(t);
                    }
                };

                /// �����׼���ͣ��ǻ������ͣ��Ķ�
                struct load_standard
                {
                    template<class T>
                    static void invoke(
                        Archive &ar, 
                        T & t)
                    {
                        LoadAccess::sub_start(ar);
                        // ֱ�ӵ���serialize����
                        using namespace util::serialization;
                        serialize(ar, t);
                        LoadAccess::sub_end(ar);
                    }
                };

                /// �����׼���ͣ��ǻ������ͣ��Ķ�
                struct load_standard_single_unit
                {
                    template<class T>
                    static void invoke(
                        Archive &ar, 
                        T & t)
                    {
                        // ֱ�ӵ���serialize����
                        using namespace util::serialization;
                        serialize(ar, t);
                    }
                };

                template<class T>
                static void invoke(
                    Archive &ar, 
                    T & t)
                {
                /// ����������𣨻������ͣ���׼���ͣ����ֱ����
                    typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                        util::serialization::is_primitive<Archive, T>, 
                        load_primitive, 
                        BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                            util::serialization::is_wrapper<T>, 
                            load_wrapper, 
                            BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                                util::serialization::is_sigle_unit<T>, 
                                load_standard_single_unit, 
                                load_standard
                            >::type
                        >::type
                    >::type invoke_type;
                    invoke_type::invoke(ar, t);
                }
            };

            /// ���ز�������>>��
            template<class T>
            Archive & operator >> (
                T const & t)
            {
                if (this->state()) return *This();
                typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                    boost::is_pointer<T>, 
                    load_pointer, 
                    BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                        boost::is_enum<T>, 
                        load_enum, 
                        load_non_pointer
                    >::type
                >::type invoke_type;
                invoke_type::invoke(*this->This(), const_cast<T &>(t));
                return *This();
            }

            /// ���ز�������&��
            template<class T>
            Archive & operator & (
                T & t)
            {
                return This()->operator >> (t);
            }

        protected:
            using BasicArchive<Archive>::This;

            /// �����ж�����׼���ַ���
            template<class _Elem, class _Traits, class _Ax>
            void load(
                std::basic_string<_Elem, _Traits, _Ax> & t)
            {
                typename std::basic_string<_Elem, _Traits, _Ax>::size_type len;
                This()->operator >> (len);
                if (this->state()) return;
                std::size_t l = 0;
                if (len > 1024) {
                    for (; len > 1024; l += 1024, len -= 1024) {
                        t.resize(l + 1024);
                        This()->load_binary(&t[l], sizeof(_Elem) * 1024);
                        if (this->state()) return;
                    }
                }
                t.resize(l + len);
                This()->load_binary(&t[l], len * sizeof(_Elem));
            }

            /// �����ж�������-ֵ��
            template<class T>
            void load_wrapper(
                util::serialization::NVPair<T> & t)
            {
                this->path_push();

                this->This()->load_start(t.name());
                This()->operator >> (t.data());
                This()->load_end(t.name());

                this->path_pop();
            }

            void load_binary(
                char * p, 
                std::size_t n)
            {
                this->state(1);
            }

            void load_start(
                std::string const & name)
            {
            }

            void load_end(
                std::string const & name)
            {
            }
        }; // class basic_iarchive

    }  // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_BASIC_I_ARCHIVE_H_

