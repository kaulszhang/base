// vector.h

#ifndef _UTIL_SERIALIZATION_STL_VECTOR_H_
#define _UTIL_SERIALIZATION_STL_VECTOR_H_

#include "util/serialization/Collection.h"
#include "util/serialization/SplitMember.h"
#include "util/serialization/Array.h"

#include <boost/mpl/if.hpp>

#include <vector>

namespace std
{

    template<
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void save(
        Archive & ar,
        std::vector<_Ty, _Alloc> const & t,
        boost::mpl::false_)
    {
        util::serialization::save_collection<Archive, std::vector<_Ty, _Alloc> >(ar, t);
    }

    template<
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void load(
        Archive & ar,
        std::vector<_Ty, _Alloc> & t,
        boost::mpl::false_)
    {
        util::serialization::load_collection<Archive, std::vector<_Ty, _Alloc> >(ar, t);
    }

    // the optimized versions

    template<
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void save(
        Archive & ar,
        std::vector<_Ty, _Alloc> const & t,
        boost::mpl::true_)
    {
        typename std::vector<_Ty, _Alloc>::size_type const count(t.size());
        ar << SERIALIZATION_NVP(count);
        if (!t.empty())
            ar.save_array(framework::container::make_array(&t[0], t.size()));
    }

    template<
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void load(
        Archive & ar,
        std::vector<_Ty, _Alloc> &t,
        boost::mpl::true_)
    {
        typename std::vector<_Ty, _Alloc>::size_type count(0);
        ar >> SERIALIZATION_NVP(count);
        t.clear();
        typename std::vector<_Ty, _Alloc>::size_type l(0);
        if (count > 1024) {
            for (; count > 1024; l += 1024, count -= 1024) {
                t.resize(l + 1024);
                ar.load_array(framework::container::make_array(&t[l], 1024));
                if (!ar) return;
            }
        }
        if (count) {
            t.resize(l + count);
            ar.load_array(framework::container::make_array(&t[l], count));
        }
    }

    // dispatch to either default or optimized versions

    template<
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void save(
        Archive & ar,
        std::vector<_Ty, _Alloc> const & t)
    {
        typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
            util::serialization::use_array_optimization<Archive, _Ty>, 
            boost::mpl::true_, 
            boost::mpl::false_>::type use_optimized;
        save(ar, t, use_optimized());
    }

    template<
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void load(
        Archive & ar,
        std::vector<_Ty, _Alloc> & t)
    {
        typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
            util::serialization::use_array_optimization<Archive, _Ty>, 
            boost::mpl::true_, 
            boost::mpl::false_>::type use_optimized;
        load(ar, t, use_optimized());
    }

    // split non-intrusive Serialize function member into separate
    // non intrusive save/load member functions
    template<
        class Archive, 
        class _Ty, 
        class _Alloc
    >
    inline void serialize(
        Archive & ar,
        std::vector<_Ty, _Alloc> & t)
    {
        util::serialization::split_free(ar, t);
    }

} // namespace std

namespace util
{
    namespace serialization
    {

        // the default versions

        namespace detail
        {
            template <
                typename _Sz
            >
            struct vector_load_max_length
            {
                static _Sz const value = 1024;
            };

            template <>
            struct vector_load_max_length<boost::uint8_t>
            {
                static boost::uint8_t const value = 128;
            };
        }

        template<
            class _Size, 
            class _Ty, 
            class _Alloc
        >
        class SizedVector
        {
        public:
            SizedVector(
                std::vector<_Ty, _Alloc> & vector)
                : vector_(vector)
            {
            }

            SERIALIZATION_SPLIT_MEMBER();

            template<class Archive>
            inline void save(
                Archive & ar,
                boost::mpl::false_) const
            {
                _Size count((_Size)vector_.size());
                ar << SERIALIZATION_NVP(count);
                save_collection<Archive, std::vector<_Ty, _Alloc>, _Size>(ar, vector_, count);
            }

            template<class Archive>
            inline void load(
                Archive & ar,
                boost::mpl::false_)
            {
                _Size count = 0;
                ar >> SERIALIZATION_NVP(count);
                load_collection<Archive, std::vector<_Ty, _Alloc>, _Size>(ar, vector_, count);
            }

            // the optimized versions

            template<class Archive>
            inline void save(
                Archive & ar,
                boost::mpl::true_) const
            {
                _Size const count((_Size)vector_.size());
                ar << SERIALIZATION_NVP(count);
                if (!vector_.empty())
                    ar << (framework::container::make_array(&vector_[0], vector_.size()));
            }

            template<class Archive>
            inline void load(
                Archive & ar,
                boost::mpl::true_)
            {
                _Size count(0);
                ar >> SERIALIZATION_NVP(count);
                vector_.clear();
                _Size l(0);
                _Size const max_length = detail::vector_load_max_length<_Size>::value;
                if (count > max_length) {
                    for (; count > max_length; l += max_length, count -= max_length) {
                        vector_.resize(l + max_length);
                        ar >> (framework::container::make_array(&vector_[l], max_length));
                        if (!ar) return;
                    }
                }
                if (count) {
                    vector_.resize(l + count);
                    ar >> framework::container::make_array(&vector_[l], count);
                }
            }

            // dispatch to either default or optimized versions

            template<class Archive>
            inline void save(
                Archive & ar) const
            {
                typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                    util::serialization::use_array_optimization<Archive, _Ty>, 
                    boost::mpl::true_, 
                    boost::mpl::false_>::type use_optimized;
                save(ar, use_optimized());
            }

            template<class Archive>
            inline void load(
                Archive & ar)
            {
                typedef BOOST_DEDUCED_TYPENAME boost::mpl::if_<
                    util::serialization::use_array_optimization<Archive, _Ty>, 
                    boost::mpl::true_, 
                    boost::mpl::false_>::type use_optimized;
                load(ar, use_optimized());
            }

        private:
            std::vector<_Ty, _Alloc> & vector_;
        };

        template<
            class _Size, 
            class _Ty, 
            class _Alloc
        >
        SizedVector<_Size, _Ty, _Alloc> const make_sized(
            std::vector<_Ty, _Alloc> & vector)
        {
            return SizedVector<_Size, _Ty, _Alloc>(vector);
        }


    } // namespace serialization
} // namespace util

#endif // _UTIL_SERIALIZATION_STL_VECTOR_H_
