// WrapObject.h

#ifndef _UTIL_DATAVIEW_WRAP_OBJECT_H_
#define _UTIL_DATAVIEW_WRAP_OBJECT_H_

#include "util/dataview/DataViewError.h"
#include "util/dataview/NodeWrap.h"
#include "util/dataview/WrapData.h"
#include "util/serialization/Serialization.h"
#include "util/serialization/NVPair.h"
using namespace util::dataview::error;
using namespace util::serialization;

//#include <framework/system/LogicError.h>

namespace util
{

    namespace dataview
    {

        template <typename Archive>
        struct ObjectArchive
        {
        public:
            ObjectArchive(
                Path const & path)
                : state_(0)
                , found_(false)
                , path_(path)
                , ec_(error_no_such_item)
            {
            }

        public:
            /// 获取派生类的指针
            Archive * This()
            {
                return static_cast<Archive *>(this);
            }

            template<class T>
            Archive & operator >> (
                T const & t)
            {
                if (state_ == 0)
                    search(t);
                return *This();
            }

            template<class T>
            Archive & operator & (
                T const & t)
            {
                return This()->operator >> (t);
            }

            template<class T>
            boost::system::error_code search_and_handle(
                char const * name, 
                T const & t)
            {
                if (path_.empty()) {
                    found_ = true;
                    return This()->handle(make_nvp(name, t));
                } else {
                    serialize(*This(), const_cast<T &>(t));
                    return ec_;
                }
            }

            template<class T>
            boost::system::error_code search_and_handle(
                T const & t)
            {
                return search_and_handle("", t);
            }

            template<class T>
            boost::system::error_code search_and_handle(
                std::string const & name, 
                T const & t)
            {
                return search_and_handle(name.c_str(), t);
            }

            template<class T>
            void enum_sub(
                NVPair<T> const & t)
            {
                serialize(*This(), const_cast<T &>(t.data()));
            }

            template<class T>
            void search(
                NVPair<T> const & t)
            {
                if (!found_) {
                    if (t.name() == path_[0]) {
                        found_ = true;
                        ec_ = This()->handle_sub(path_.sub_path(), t, 
                            (NodeTypeT<NodeWrap<T>::node_type::node_type_enum> *)NULL);
                    }
                } else {
                    This()->handle_item(t);
                }
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_cat> *)
            {
                return error_unsupported;
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_data> *)
            {
                return error_unsupported;
            }

            template<class T>
            void handle_item(
                NVPair<T> const & t)
            {
                state_ = 2;
            }

            operator bool () const
            {
                return state_ == 0;
            }

            bool operator ! () const
            {
                return state_ != 0;
            }

        private:
            int state_;
            bool found_;
            Path const & path_;
            boost::system::error_code ec_;
        };

        struct ObjectArchiveGetChildrenNum
            : ObjectArchive<ObjectArchiveGetChildrenNum>
        {
            ObjectArchiveGetChildrenNum(
                Path const & path, 
                size_t & n)
                : ObjectArchive<ObjectArchiveGetChildrenNum>(path)
                , n_(n)
            {
                n_ = 0;
            }

            template<class T>
            boost::system::error_code handle(
                NVPair<T> const & t)
            {
                enum_sub(t);
                return error_succeed;
            }

            template<class T>
            void handle_item(
                NVPair<T> const & t)
            {
                ++n_;
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_cat> *)
            {
                return typename NodeWrap<T>::node_type(t.name(), t.data()).get_children_num(path, n_);
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_data> *)
            {
                // get_children_num 需要目录类型节点
                return error_invalid_param;
            }

        private:
            size_t & n_;
        };

        struct ObjectArchiveGetChildren
            : ObjectArchive<ObjectArchiveGetChildren>
        {
            ObjectArchiveGetChildren(
                Path const & path, 
                std::vector<Node> & children)
                : ObjectArchive<ObjectArchiveGetChildren>(path)
                , children_(children)
            {
            }

            template<class T>
            boost::system::error_code handle(
                NVPair<T> const & t)
            {
                enum_sub(t);
                return error_succeed;
            }

            template<class T>
            void handle_item(
                NVPair<T> const & t)
            {
                children_.push_back(Node(t.name(), NodeWrap<T>::node_type::node_type_enum));
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_cat> *)
            {
                return typename NodeWrap<T>::node_type(t.name(), t.data()).get_children(path, children_);
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_data> *)
            {
                // get_children 需要目录类型节点
                return error_invalid_param;
            }

        private:
            std::vector<Node> & children_;
        };

        struct ObjectArchiveGetChildDataType
            : ObjectArchive<ObjectArchiveGetChildDataType>
        {
            ObjectArchiveGetChildDataType(
                Path const & path, 
                DataType & data_type)
                : ObjectArchive<ObjectArchiveGetChildDataType>(path)
                , data_type_(data_type)
            {
            }

            template<class T>
            boost::system::error_code handle(
                NVPair<T> const & t)
            {
                // get_children_data_type 需要指定子节点
                return error_invalid_param;
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_cat> *)
            {
                // get_children_data_type 需要子节点是数值类型
                return error_invalid_param;
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_data> *)
            {
                // 如果最终找到，又是数值类型，就是他了
                if (path.empty()) {
                    data_type_ = NodeWrap<T>::node_type::data_type_enum;
                    return error_succeed;
                } else {
                    // 不能再递归到更深层了
                    return error_invalid_param;
                }
            }

        private:
            DataType & data_type_;
        };

        struct ObjectArchiveGetChildData
            : ObjectArchive<ObjectArchiveGetChildData>
        {
            ObjectArchiveGetChildData(
                Path const & path, 
                Data & data)
                : ObjectArchive<ObjectArchiveGetChildData>(path)
                , data_(data)
            {
            }

            template<class T>
            boost::system::error_code handle(
                NVPair<T> const & t)
            {
                // get_children_datae 需要指定子节点
                return error_invalid_param;
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_cat> *)
            {
                // get_children_data 需要子节点是数值类型
                return error_invalid_param;
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_data> *)
            {
                // 如果最终找到，又是数值类型，就是他了
                if (path.empty()) {
                    data_ = typename NodeWrap<T>::node_type(t.name(), t.data()).get_data();
                    return error_succeed;
                } else {
                    // 不能再递归到更深层了
                    return error_invalid_param;
                }
            }

        private:
            Data & data_;
        };

        struct ObjectArchiveGetTableHead
            : ObjectArchive<ObjectArchiveGetTableHead>
        {
            ObjectArchiveGetTableHead(
                Path const & path, 
                TableHead & table_head)
                : ObjectArchive<ObjectArchiveGetTableHead>(path)
                , table_head_(table_head)
            {
            }

            template<class T>
            boost::system::error_code handle(
                NVPair<T> const & t)
            {
                table_head_.item_type = Node::t_data;
                table_head_.column_child = true;
                table_head_.item_count = 1;
                enum_sub(t);
                return error_succeed;
            }

            template<class T>
            void handle_item(
                NVPair<T> const & t)
            {
                handle_item(t, (NodeTypeT<NodeWrap<T>::node_type::node_type_enum> *)NULL);
            }

            template<class T>
            void handle_item(
                NVPair<T> const & t, 
                NodeTypeT<Node::t_cat> *)
            {
            }

            template<class T>
            void handle_item(
                NVPair<T> const & t, 
                NodeTypeT<Node::t_data> *)
            {
                table_head_.columns.push_back(ColumnHead(t.name(), NodeWrap<T>::node_type::data_type_enum));
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_cat> *)
            {
                return typename NodeWrap<T>::node_type(t.name(), t.data()).get_table_head(path, table_head_);
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_data> *)
            {
                // get_table_head 需要目录类型节点
                return error_invalid_param;
            }

        private:
            TableHead & table_head_;
        };

        struct ObjectArchiveGetTableData
            : ObjectArchive<ObjectArchiveGetTableData>
        {
            ObjectArchiveGetTableData(
                Path const & path, 
                size_t from, 
                size_t size, 
                TableData & table_data)
                : ObjectArchive<ObjectArchiveGetTableData>(path)
                , from_(from)
                , size_(size)
                , table_data_(table_data)
            {
            }

            template<class T>
            boost::system::error_code handle(
                NVPair<T> const & t)
            {
                table_data_.count = 1;
                table_data_.items.push_back("属性");
                enum_sub(t);
                return error_succeed;
            }

            template<class T>
            void handle_item(
                NVPair<T> const & t)
            {
                handle_item(t, (NodeTypeT<NodeWrap<T>::node_type::node_type_enum> *)NULL);
            }

            template<class T>
            void handle_item(
                NVPair<T> const & t, 
                NodeTypeT<Node::t_cat> *)
            {
            }

            template<class T>
            void handle_item(
                NVPair<T> const & t, 
                NodeTypeT<Node::t_data> *)
            {
                table_data_.datas.push_back(ColumnData(typename NodeWrap<T>::node_type(t.name(), t.data()).get_data()));
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_cat> *)
            {
                return typename NodeWrap<T>::node_type(t.name(), t.data()).get_table_data(path, from_, size_, table_data_);
            }

            template<class T>
            boost::system::error_code handle_sub(
                Path const & path, 
                NVPair<T> const & t, 
                NodeTypeT<Node::t_data> *)
            {
                // get_table_data 需要目录类型节点
                return error_invalid_param;
            }

        private:
            size_t from_;
            size_t size_;
            TableData & table_data_;
        };

        template <typename T>
        struct ObjectNodeT
            : CatalogNode
        {
            static bool const single_line = true;

            ObjectNodeT(
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
                return ObjectArchiveGetChildrenNum(path, n).search_and_handle(name, t_);
            }

            virtual boost::system::error_code get_children(
                Path const & path, 
                std::vector<Node> & children)
            {
                return ObjectArchiveGetChildren(path, children).search_and_handle(name, t_);
            }

            virtual boost::system::error_code get_child_data_type(
                Path const & path, 
                DataType & data_type)
            {
                return ObjectArchiveGetChildDataType(path, data_type).search_and_handle(name, t_);
            }

            virtual boost::system::error_code get_child_data(
                Path const & path, 
                Data & data)
            {
                return ObjectArchiveGetChildData(path, data).search_and_handle(name, t_);
            }

            virtual boost::system::error_code get_table_head(
                Path const & path, 
                TableHead & table_head)
            {
                return ObjectArchiveGetTableHead(path, table_head).search_and_handle(name, t_);
            }

            virtual boost::system::error_code get_table_data(
                Path const & path, 
                size_t from, 
                size_t size, 
                TableData & table_data)
            {
                return ObjectArchiveGetTableData(path, from, size, table_data).search_and_handle(name, t_);
            }

        private:
            T const & t_;
        };

        template <typename T>
        struct WrapObject
        {
            typedef ObjectNodeT<T> node_type;
        };

        template <>
        struct WrapObject<text>
        {
            typedef DataNodeT<text, text> node_type;
        };
    }
}

#endif // _UTIL_DATAVIEW_WRAP_OBJECT_H_
