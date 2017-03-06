// NodeWrap.h

#ifndef _UTIL_DATAVIEW_NODE_WRAP_H_
#define _UTIL_DATAVIEW_NODE_WRAP_H_

#include "util/dataview/CatalogNode.h"
#include "util/dataview/DataNode.h"

#include <framework/string/Format.h>

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_pointer.hpp>

namespace util
{

    namespace dataview
    {
        template<typename T>
        std::string to_string(T const & t)
        {
            return framework::string::format(t);
        }

        template <Node::TypeEnum E>
        struct NodeTypeT
        {
        };

        template <typename T>
        struct NodeWrap;

        template <typename T, typename F>
        struct WrapData;

        template <typename T>
        struct WrapObject;

        template <typename T>
        struct is_sigle_unit
            : boost::mpl::false_
        {
        };

        struct NodeWrapNoPointer
        {
            template <typename T>
            struct apply
            {
                typedef typename boost::mpl::if_<
                    boost::is_enum<T>, 
                    typename WrapData<T, int>::node_type, 
                    typename boost::mpl::if_<
                    boost::is_fundamental<T>, 
                        typename boost::mpl::if_<
                        boost::is_integral<T>, 
                            typename WrapData<T, int>::node_type, 
                            typename WrapData<T, float>::node_type
                        >::type, 
                        typename boost::mpl::if_<
                            is_sigle_unit<T>, 
                            typename WrapObject<T>::node_type, 
                            typename WrapObject<T>::node_type
                        >::type
                    >::type
                >::type type;
            };
        };

        template <
            typename T, 
            typename NT
        >
        struct PointerNode;

        template <typename T>
        struct IsPointer
            : boost::is_pointer<T>
        {
        };

        template <typename T>
        struct PointerValueType
        {
            typedef typename T::value_type type;
        };

        template <typename T>
        struct PointerValueType<T *>
        {
            typedef typename boost::remove_cv<typename boost::remove_pointer<T>::type>::type type;
        };

        template <
            typename T
        >
        struct PointerNode<T, NodeTypeT<Node::t_cat> >
            : CatalogNode
        {
            typedef typename PointerValueType<T>::type naked_type;
            typedef typename NodeWrap<naked_type>::node_type naked_node_type;
            static bool const single_line = naked_node_type::single_line;

            PointerNode(
                std::string const & name, 
                T const & t)
                : CatalogNode(name)
                , t_(t)
            {
            }

            virtual boost::system::error_code get_children_num(
                Path const & path, 
                size_t & n)
            {
                if (!t_)
                    return ERROR_TRY_AGAIN;
                return naked_node_type(name, *t_).get_children_num(path, n);
            }

            virtual boost::system::error_code get_children(
                Path const & path, 
                std::vector<Node> & children)
            {
                if (!t_)
                    return ERROR_TRY_AGAIN;
                return naked_node_type(name, *t_).get_children(path, children);
            }

            virtual boost::system::error_code get_child_data_type(
                Path const & path, 
                DataType & data_type)
            {
                if (!t_)
                    return ERROR_TRY_AGAIN;
                return naked_node_type(name, *t_).get_child_data_type(path, data_type);
            }

            virtual boost::system::error_code get_child_data(
                Path const & path, 
                Data & data)
            {
                if (!t_)
                    return ERROR_TRY_AGAIN;
                return naked_node_type(name, *t_).get_child_data(path, data);
            }

            virtual boost::system::error_code get_table_head(
                Path const & path, 
                TableHead & table_head)
            {
                if (!t_)
                    return ERROR_TRY_AGAIN;
                return naked_node_type(name, *t_).get_table_head(path, table_head);
            }

            virtual boost::system::error_code get_table_data(
                Path const & path, 
                size_t from, 
                size_t size, 
                TableData & table_data)
            {
                if (!t_)
                    return ERROR_TRY_AGAIN;
                return naked_node_type(name, *t_).get_table_data(path, from, size, table_data);
            }

            T const & t_;
        };

        template <
            typename T
        >
        struct PointerNode<T, NodeTypeT<Node::t_data> >
            : DataNode
        {
            typedef typename PointerValueType<T>::type naked_type;
            typedef typename NodeWrap<naked_type>::node_type naked_node_type;

            PointerNode(
                std::string const & name, 
                T const & t)
                : CatalogNode(name)
                , t_(t)
            {
            }

            virtual void get_data_type(
                DataType & data_type)
            {
                if (!t_)
                    return ERROR_TRY_AGAIN;
                return naked_node_type(name, *t_).get_data_type(data_type);
            }

            virtual void get_data(
                Data & data)
            {
                if (!t_)
                    return ERROR_TRY_AGAIN;
                return naked_node_type(name, *t_).get_data(data);
            }

            T const & t_;
        };

        struct NodeWrapPointer
        {
            template <typename T>
            struct apply
            {
                typedef typename PointerValueType<T>::type naked_type;
                typedef typename NodeWrap<naked_type>::node_type naked_node_type;
                typedef PointerNode<T, NodeTypeT<naked_node_type::node_type_enum> > type;
            };
        };

        template <typename T>
        struct NodeWrap
        {
            typedef typename boost::mpl::if_<
                IsPointer<T>, 
                NodeWrapPointer, 
                NodeWrapNoPointer
            >::type wrap_type;
            typedef typename wrap_type::template apply<T>::type node_type;
        };

        template <typename T>
        typename NodeWrap<T>::node_type wrap_node(
            std::string const & name, 
            T const & t)
        {
            return typename NodeWrap<T>::node_type(name, t);
        }

        template <typename T>
        typename NodeWrap<T>::node_type * new_wrap_node(
            std::string const & name, 
            T const & t)
        {
            return new typename NodeWrap<T>::node_type(name, t);
        }
    }
}

#endif // _UTIL_DATAVIEW_NODE_WRAP_H_
