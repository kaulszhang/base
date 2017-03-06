// CatalogNode.cpp

#include "util/Util.h"
#include "util/dataview/CatalogNode.h"
#include "util/dataview/DataViewError.h"
using namespace util::dataview::error;

using namespace boost::system;

namespace util
{

    namespace dataview
    {
        error_code CatalogNode::get_table_head(
            Path const & path, 
            TableHead & table_head)
        {
            size_t num;
            error_code ret = get_children_num(path, num);
            if (ret == error_succeed && num > 10) {
                ret = error_unsupported;
            }
            if (ret == error_succeed) {
                std::vector<Node> children;
                ret = get_children(path, children);
                for (size_t i = 0; ret == error_succeed && i < children.size(); ++i) {
                    // 只要有一个子节点不是数值型，不处理
                    if (children[i].type == t_cat) {
                        ret = error_unsupported;
                    }
                    DataType dt;
                    get_child_data_type(path / children[i].name, dt);
                    table_head.columns.push_back(ColumnHead(children[i].name, dt));
                    table_head.item_count = 1;
                }
            }
            return ret;
        }

        error_code CatalogNode::get_table_data(
            Path const & path, 
            size_t from, 
            size_t to, 
            TableData & table)
        {
            size_t num;
            error_code ret = get_children_num(path, num);
            if (ret == error_succeed && num > 10) {
                ret = error_unsupported;
            }
            table.count = 1;
            if (ret == error_succeed && from == 0 && to >= 1) {
                std::vector<Node> children;
                ret = get_children(path, children);
                for (size_t i = 0; ret == error_succeed && i < children.size(); ++i) {
                    // 只要有一个子节点不是数值型，不处理
                    if (children[i].type == t_cat) {
                        ret = error_unsupported;
                    }
                    Data d;
                    get_child_data(path / children[i].name, d);
                    table.datas.push_back(ColumnData(d));
                }
            }
            return ret;
        }

    }

}
