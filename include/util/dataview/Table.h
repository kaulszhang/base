// Table.h

#ifndef _UTIL_DATAVIEW_TABLE_H_
#define _UTIL_DATAVIEW_TABLE_H_

#include "util/dataview/Data.h"
#include "util/dataview/Node.h"

namespace util
{

    namespace dataview
    {
        struct ColumnHead
        {
            ColumnHead(
                DataType const & dt = DataType::t_nodata)
                : type(dt)
            {
            }

            ColumnHead(
                std::string const & name, 
                DataType const & dt = DataType::t_nodata)
                : name(name)
                , type(dt)
            {
            }

            ColumnHead(
                char const * name, 
                DataType const & dt = DataType::t_nodata)
                : name(name)
                , type(dt)
            {
            }

            std::string name;
            DataType type;

            void swap(
                ColumnHead & r)
            {
                type.swap(r.type);
                name.swap(r.name);
            }

            void clear()
            {
                type.clear();
                name.clear();
            }

            template<typename Archive>
            void serialize(Archive & ar);
        };

        struct TableHead
        {
            std::vector<ColumnHead> columns;
            Node::TypeEnum item_type;
            bool column_child;
            size_t item_count;

            TableHead()
                : item_type(Node::t_cat)
                , column_child(false)
                , item_count(0)
            {
            }

            void swap(
                TableHead & r)
            {
                columns.swap(r.columns);
                std::swap(item_type, r.item_type);
                std::swap(column_child, r.column_child);
                std::swap(item_count, r.item_count);
            }

            void clear()
            {
                columns.clear();
                item_type = Node::t_cat;
                column_child = false;
                item_count = 0;
            }

            template<typename Archive>
            void serialize(Archive & ar);
        };

        struct ColumnData
        {
            DataType type;

            std::vector<bool> vec_bool;
            std::vector<int> vec_int;
            std::vector<float> vec_float;
            std::vector<text> vec_text;

            ColumnData(
                DataType::TypeEnum dte = DataType::t_nodata) // column with no item
                : type(dte)
            {
            }

            ColumnData(
                Data const & d) // column with one item
                : type(d.type)
            {
                switch (type.type) {
                    case DataType::t_bool:
                        vec_bool.push_back(d.v_bool);
                        break;
                    case DataType::t_int:
                        vec_int.push_back(d.v_int);
                        break;
                    case DataType::t_float:
                        vec_float.push_back(d.v_float);
                        break;
                    case DataType::t_text:
                        vec_text.push_back(d.v_text);
                        break;
                    default:
                        break;
                }
            }

            void push(
                Data const & d)
            {
                assert(type == d.type);
                switch (type.type) {
                    case DataType::t_bool:
                        vec_bool.push_back(d.v_bool);
                        break;
                    case DataType::t_int:
                        vec_int.push_back(d.v_int);
                        break;
                    case DataType::t_float:
                        vec_float.push_back(d.v_float);
                        break;
                    case DataType::t_text:
                        vec_text.push_back(d.v_text);
                        break;
                    default:
                        break;
                }
            }

            void append(
                ColumnData const & c)
            {
                assert(type == c.type);
                switch (type.type) {
                    case DataType::t_bool:
                        vec_bool.insert(vec_bool.end(), c.vec_bool.begin(), c.vec_bool.end());
                        break;
                    case DataType::t_int:
                        vec_int.insert(vec_int.end(), c.vec_int.begin(), c.vec_int.end());
                        break;
                    case DataType::t_float:
                        vec_float.insert(vec_float.end(), c.vec_float.begin(), c.vec_float.end());
                        break;
                    case DataType::t_text:
                        vec_text.insert(vec_text.end(), c.vec_text.begin(), c.vec_text.end());
                        break;
                    default:
                        break;
                }
            }

            void swap(
                ColumnData & r)
            {
                type.swap(r.type);
                vec_bool.swap(r.vec_bool);
                vec_int.swap(r.vec_int);
                vec_float.swap(r.vec_float);
                vec_text.swap(r.vec_text);
            }

            void clear()
            {
                type.clear();
                vec_bool.clear();
                vec_int.clear();
                vec_float.clear();
                vec_text.clear();
            }

            template<typename Archive>
            void serialize(Archive & ar);
        };

        struct ItemHead
        {
            ItemHead()
            {
            }

            ItemHead(
                std::string const & name)
                : name(name)
            {
            }

            ItemHead(
                char const * name)
                : name(name)
            {
            }

            std::string name;

            friend bool operator < (
                ItemHead const & l, 
                ItemHead const & r)
            {
                return l.name < r.name;
            }

            friend bool operator == (
                ItemHead const & l, 
                ItemHead const & r)
            {
                return l.name == r.name;
            }

            friend bool operator != (
                ItemHead const & l, 
                ItemHead const & r)
            {
                return l.name != r.name;
            }

            template<typename Archive>
            void serialize(Archive & ar);
        };

        struct ItemData
        {
            std::vector<Data> datas;

            void swap(
                ItemData & r)
            {
                datas.swap(r.datas);
            }

            void clear()
            {
                datas.clear();
            }

            template<typename Archive>
            void serialize(Archive & ar);
        };

        struct TableData
        {
            size_t count;
            std::vector<ItemHead> items;
            std::vector<ColumnData> datas;

            void swap(
                TableData & r)
            {
                std::swap(count, r.count);
                items.swap(r.items);
                datas.swap(r.datas);
            }

            void clear()
            {
                count = 0;
                items.clear();
                datas.clear();
            }

            void append(
                TableData const & t)
            {
                if (datas.size() == 0) {
                    items = t.items;
                    datas = t.datas;
                    return;
                }
                assert(datas.size() == t.datas.size());
                items.insert(items.end(), t.items.begin(), t.items.end());
                for (size_t i = 0; i < datas.size(); ++i) {
                    datas[i].append(t.datas[i]);
                }
            }

            template<typename Archive>
            void serialize(Archive & ar);
        };
    }
}

#endif // _UTIL_DATAVIEW_TABLE_H_
