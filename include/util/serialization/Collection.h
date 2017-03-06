// Collection.h

#ifndef _UTIL_SERIALIZATION_COLLECTION_H_
#define _UTIL_SERIALIZATION_COLLECTION_H_

#include "util/serialization/NVPair.h"
#include "util/serialization/SplitMember.h"

namespace util
{
    namespace serialization
    {

        /// 调用类中拆分出来的序列化方法
        template<
            class Archive, 
            class Container
        >
        inline void save_collection(
            Archive & ar, 
            Container const & t)
        {
            typename Container::size_type count(t.size());
            ar <<  SERIALIZATION_NVP(count);
            typename Container::const_iterator it = t.begin();
            while(ar && count-- > 0) {
                ar << util::serialization::make_nvp("item", *it++);
            }
        }

        template<
            class Archive, 
            class Container
        >
        inline void load_collection(
            Archive & ar, 
            Container & t)
        {
            t.clear();
            typename Container::size_type count = 0;
            ar >>  SERIALIZATION_NVP(count);
            if (!ar)
                return;
            while(count-- > 0) {
                typename Container::value_type v;
                ar >> util::serialization::make_nvp("item", v);
                if (ar)
                    t.insert(t.end(), v);
                else
                    break;
            }
        }

        template<
            class Archive, 
            class Container, 
            class Size
        >
        inline void save_collection(
            Archive & ar, 
            Container const & t, 
            Size n)
        {
            typename Container::const_iterator it = t.begin();
            Size count = n;
            while(ar && count-- > 0) {
                ar << util::serialization::make_nvp("item", *it++);
            }
        }

        template<
            class Archive, 
            class Container, 
            class Size
        >
        inline void load_collection(
            Archive & ar, 
            Container & t, 
            Size n)
        {
            t.clear();
            Size count = n;
            while(count-- > 0) {
                typename Container::value_type v;
                ar >> util::serialization::make_nvp("item", v);
                if (ar)
                    t.insert(t.end(), v);
                else
                    break;
            }
        }

        struct free_collection_saver
        {
            template <
                class Archive, 
                class T
            >
            static void invoke(
                Archive & ar, 
                T const & t)
            {
                save_collection(ar, t);
            }

            template <
                class Archive, 
                class T, 
                class Size
            >
            static void invoke(
                Archive & ar, 
                T const & t, 
                Size n)
            {
                save_collection(ar, t, n);
            }
        };

        /// 调用类中拆分出来的反序列化方法
        struct free_collection_loader
        {
            template<
                class Archive, 
                class T
            >
            static void invoke(
                Archive & ar, 
                T & t)
            {
                load_collection(ar, t);
            }

            template<
                class Archive, 
                class T, 
                class Size
            >
            static void invoke(
                Archive & ar, 
                T & t, 
                Size n)
            {
                load_collection(ar, t, n);
            }
        };
        
        template<
            class Archive, 
            class Container
        >
        inline void serialize_collection(
            Archive & ar, 
            Container & t)
        {
            typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                BOOST_DEDUCED_TYPENAME Archive::is_saving, 
                free_collection_saver, 
                free_collection_loader
            >::type typex;
            typex::invoke(ar, t);
        }

        template<
            class Archive, 
            class Container, 
            class Size
        >
        inline void serialize_collection(
            Archive & ar, 
            Container & t, 
            Size n)
        {
            typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                BOOST_DEDUCED_TYPENAME Archive::is_saving, 
                free_collection_saver, 
                free_collection_loader
            >::type typex;
            typex::invoke(ar, t, n);
        }

        template<
            class Size, 
            class Container
        >
        class Sized
        {
        public:
            Sized(
                Container & c)
                : c_(c)
            {
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void save(Archive & ar) const
            {
                Size size = c_.size();
                ar & size;
                save_collection(ar, c_, size);
            }

            template <typename Archive>
            void load(Archive & ar)
            {
                Size size = 0;
                ar & size;
                load_collection(ar, c_, size);
            }

        private:
            Container & c_;
        };

        template<
            class Size, 
            class Container
        >
        Sized<Size, Container> const make_sized(
            Container & c)
        {
            return Sized<Size, Container>(c);
        }

    } // namespace serialization
} // namespace util

#endif // _UTIL_SERIALIZATION_COLLECTION_H_
