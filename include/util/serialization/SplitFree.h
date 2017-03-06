// SplitFree.h

#ifndef _UTIL_SERIALIZATION_SPLIT_FREE_H_
#define _UTIL_SERIALIZATION_SPLIT_FREE_H_

namespace util
{
    namespace serialization
    {

        /// �������в�ֳ��������л�����
        template<class Archive, class T>
        struct free_saver {
            static void invoke(
                Archive & ar, 
                T const & t)
            {
                save(ar, t);
            }
        };

        /// �������в�ֳ����ķ����л�����
        template<class Archive, class T>
        struct free_loader {
            static void invoke(
                Archive & ar, 
                T & t)
            {
                load(ar, t);
            }
        };

        /// ���������л��������л�������
        /// ����Archive�����ͷֱ����save��load����
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
