// SplitFree.h

#ifndef _UTIL_SERIALIZATION_SPLIT_FREE_H_
#define _UTIL_SERIALIZATION_SPLIT_FREE_H_

namespace util
{
    namespace serialization
    {

        /// 调用类中拆分出来的序列化方法
        template<class Archive, class T>
        struct free_saver {
            static void invoke(
                Archive & ar, 
                T const & t)
            {
                save(ar, t);
            }
        };

        /// 调用类中拆分出来的反序列化方法
        template<class Archive, class T>
        struct free_loader {
            static void invoke(
                Archive & ar, 
                T & t)
            {
                load(ar, t);
            }
        };

        /// 拆分类的序列化（反序列化）方法
        /// 根据Archive的类型分别调用save，load方法
        template<class Archive, class T>
        inline void split_free(
            Archive & ar, T & t)
        {
            typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                BOOST_DEDUCED_TYPENAME Archive::is_saving, 
                free_saver<Archive, T>, 
                free_loader<Archive, T>
            >::type typex;
            typex::invoke(ar, t);
        }

    }  // namespace serialization
} // namespace util

#endif // _UTIL_SERIALIZATION_SPLIT_FREE_H_
