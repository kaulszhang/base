// basic_oarchive.h

#ifndef _UTIL_ARCHIVE_BASIC_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_BASIC_O_ARCHIVE_H_

#include "util/archive/BasicArchive.h"

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_enum.hpp>

namespace util
{
    namespace archive
    {

        struct SaveAccess
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
            static void save_wrapper(Archive & ar, T const & t)
            {
                ar.save_wrapper(t);
            }
        };

        /**
            ����ĵ��Ļ���
            
            �ṩ��<<������&������������
            ����Դ������������׼���͵����л�
            @param Archive ������basic_oarchive������ĵ��࣬ʵ�ֶԻ������͵����
         */
        template <typename Archive>
        class BasicOArchive
            : public BasicArchive<Archive>
        {
        public:
            typedef boost::mpl::true_ is_saving; ///< �����ù鵵�����ڱ������ݵ�
            typedef boost::mpl::false_ is_loading; ///< �����ù鵵�����ǡ����ڼ������ݵ�
            friend struct SaveAccess;

        public:
            /// ����ָ�����л�
            struct save_pointer
            {
                template<class T>
                static void invoke(
                    Archive & ar, 
                    T const & t)
                {
                    // ֱ�ӵ���Archive��save������ͨ��save_access��ת
                    ar.operator << (*t);
                }
            };

            /// �����������л�
            struct save_array
            {
                template<class T>
                static void invoke(
                    Archive & ar, 
                    T const & t)
                {
                    int count = sizeof(t) / (
                        static_cast<const char *>(static_cast<const void *>(&t[1])) 
                        - static_cast<const char *>(static_cast<const void *>(&t[0]))
                        );
                    ar.operator << (SERIALIZATION_NVP(count));
                    ar.operator << (make_array(static_cast<T const *>(&t[0]), count));
                }
            };

            /// ����ָ�����л�
            struct save_enum
            {
                template<class T>
                static void invoke(
                    Archive & ar, 
                    T const & t)
                {
                    const int i = static_cast<int>(t);
                    ar.save(i);
                }
            };

            struct save_non_pointer
            {
                /// �����װ�������л�
                struct save_wrapper
                {
                    template<class T>
                    static void invoke(
                        Archive & ar, 
                        T const & t)
                    {
                        // ֱ�ӵ���Archive��save_wrapper������ͨ��save_access��ת
                        SaveAccess::save_wrapper(ar, t);
                    }
                };

                /// ��������������л�
                struct save_primitive
                {
                    template<class T>
                    static void invoke(
                        Archive & ar, 
                        T const & t)
                    {
                        // ֱ�ӵ���Archive��save������ͨ��save_access��ת
                        ar.save(t);
                    }
                };

                /// �����׼���ͣ��ǻ������ͣ����л�
                struct save_standard
                {
                    template<class T>
                    static void invoke(
                        Archive & ar, 
                        T const & t)
                    {
                        // ֱ�ӵ���serialize����
                        SaveAccess::sub_start(ar);
                        using namespace util::serialization;
                        serialize(ar, const_cast<T &>(t));
                        SaveAccess::sub_end(ar);
                    }
                };

                /// �����׼���ͣ��ǻ������ͣ����л�
                struct save_standard_single_unit
                {
                    template<class T>
                    static void invoke(
                        Archive & ar, 
                        T const & t)
                    {
                        // ֱ�ӵ���serialize����
                        using namespace util::serialization;
                        serialize(ar, const_cast<T &>(t));
                    }
                };

                template<class T>
                static void invoke(
                    Archive & ar, 
                    T const & t)
                {
                    /// ����������𣨻������ͣ���׼���ͣ����ֱ������л�
                    typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                        util::serialization::is_primitive<Archive, T>, 
                        save_primitive, 
                        BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                            util::serialization::is_wrapper<T>, 
                            save_wrapper, 
                            BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                                util::serialization::is_sigle_unit<T>, 
                                save_standard_single_unit, 
                                save_standard
                            >::type
                        >::type
                    >::type invoke_type;
                    invoke_type::invoke(ar, t);
                }

            };
            /// ���ز�������<<��
            template<class T>
            Archive & operator << (
                T const & t)
            {
                /// ����������𣨻������ͣ���׼���ͣ����ֱ������л�
                typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                    boost::is_pointer<T>, 
                    save_pointer, 
                    BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                        boost::is_enum<T>, 
                        save_enum, 
                        save_non_pointer
                    >::type
                >::type invoke_type;
                invoke_type::invoke(*This(), t);
                return *This();
            }

            /// ���ز�������&��
            template<class T>
            Archive & operator & (
                T const & t)
            {
                This()->operator << (t);
                return *This();
            }

        protected:
            using BasicArchive<Archive>::This;

            /// ������д���׼���ַ���
            template<class _Elem, class _Traits, class _Ax>
            void save(
                std::basic_string<_Elem, _Traits, _Ax> const & t)
            {
                typename std::basic_string<_Elem, _Traits, _Ax>::size_type size = t.size();
                This()->operator << (size);
                This()->save_binary((char const *)&t[0], t.size() * sizeof(_Elem));
            }

            /// ������д������-ֵ��
            template<class T>
            void save_wrapper(
                util::serialization::NVPair<T> const & t)
            {
                this->path_push();

                This()->save_start(t.name());
                This()->operator << (t.data());
                This()->save_end(t.name());

                this->path_pop();
            }

            void save_binary(
                char const * p, 
                std::size_t n)
            {
                this->state(1);
            }

            void save_start(
                std::string const & name)
            {
            }

            void save_end(
                std::string const & name)
            {
            }
        };

    } // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_BASIC_O_ARCHIVE_H_
