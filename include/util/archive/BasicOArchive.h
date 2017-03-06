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
            输出文档的基类
            
            提供“<<”、“&”操作符重载
            区别对待基本类型与标准类型的序列化
            @param Archive 派生自basic_oarchive的输出文档类，实现对基本类型的输出
         */
        template <typename Archive>
        class BasicOArchive
            : public BasicArchive<Archive>
        {
        public:
            typedef boost::mpl::true_ is_saving; ///< 表明该归档是用于保存数据的
            typedef boost::mpl::false_ is_loading; ///< 表明该归档“不是”用于加载数据的
            friend struct SaveAccess;

        public:
            /// 处理指针序列化
            struct save_pointer
            {
                template<class T>
                static void invoke(
                    Archive & ar, 
                    T const & t)
                {
                    // 直接调用Archive的save方法，通过save_access中转
                    ar.operator << (*t);
                }
            };

            /// 处理数组序列化
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

            /// 处理指针序列化
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
                /// 处理包装类型序列化
                struct save_wrapper
                {
                    template<class T>
                    static void invoke(
                        Archive & ar, 
                        T const & t)
                    {
                        // 直接调用Archive的save_wrapper方法，通过save_access中转
                        SaveAccess::save_wrapper(ar, t);
                    }
                };

                /// 处理基本类型序列化
                struct save_primitive
                {
                    template<class T>
                    static void invoke(
                        Archive & ar, 
                        T const & t)
                    {
                        // 直接调用Archive的save方法，通过save_access中转
                        ar.save(t);
                    }
                };

                /// 处理标准类型（非基本类型）序列化
                struct save_standard
                {
                    template<class T>
                    static void invoke(
                        Archive & ar, 
                        T const & t)
                    {
                        // 直接调用serialize方法
                        SaveAccess::sub_start(ar);
                        using namespace util::serialization;
                        serialize(ar, const_cast<T &>(t));
                        SaveAccess::sub_end(ar);
                    }
                };

                /// 处理标准类型（非基本类型）序列化
                struct save_standard_single_unit
                {
                    template<class T>
                    static void invoke(
                        Archive & ar, 
                        T const & t)
                    {
                        // 直接调用serialize方法
                        using namespace util::serialization;
                        serialize(ar, const_cast<T &>(t));
                    }
                };

                template<class T>
                static void invoke(
                    Archive & ar, 
                    T const & t)
                {
                    /// 根据类型类别（基本类型，标准类型），分别处理序列化
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
            /// 重载操作符“<<”
            template<class T>
            Archive & operator << (
                T const & t)
            {
                /// 根据类型类别（基本类型，标准类型），分别处理序列化
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

            /// 重载操作符“&”
            template<class T>
            Archive & operator & (
                T const & t)
            {
                This()->operator << (t);
                return *This();
            }

        protected:
            using BasicArchive<Archive>::This;

            /// 向流中写入标准库字符串
            template<class _Elem, class _Traits, class _Ax>
            void save(
                std::basic_string<_Elem, _Traits, _Ax> const & t)
            {
                typename std::basic_string<_Elem, _Traits, _Ax>::size_type size = t.size();
                This()->operator << (size);
                This()->save_binary((char const *)&t[0], t.size() * sizeof(_Elem));
            }

            /// 向流中写入名字-值对
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
