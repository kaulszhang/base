// SplitMember.h

#ifndef _UTIL_SERIALIZATION_SPLIT_MEMBER_H_
#define _UTIL_SERIALIZATION_SPLIT_MEMBER_H_

namespace util
{
    namespace serialization
    {

        /// �������в�ֳ��������л�����
        template<class Archive, class T>
        struct member_saver {
            static void invoke(
                Archive & ar, 
                T const & t)
            {
                t.save(ar);
            }
        };

        /// �������в�ֳ����ķ����л�����
        template<class Archive, class T>
        struct member_loader {
            static void invoke(
                Archive & ar, 
                T & t)
            {
                t.load(ar);
            }
        };

        /// ���������л��������л�������
        /// ����Archive�����ͷֱ����save��load����
        template<class Archive, class T>
        inline void split_member(
            Archive & ar, T & t)
        {
            typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                BOOST_DEDUCED_TYPENAME Archive::is_saving, 
                member_saver<Archive, T>, 
                member_loader<Archive, T>
            >::type typex;
            typex::invoke(ar, t);
        }

    }  // namespace serialization
} // namespace util

/// ����serialize��ֹ��ܵĺ�
#define SERIALIZATION_SPLIT_MEMBER() \
    template<class Archive> \
    void serialize( \
    Archive & ar) \
{ \
    util::serialization::split_member(ar, *this); \
}

#endif // _UTIL_SERIALIZATION_SPLIT_MEMBER_H_
