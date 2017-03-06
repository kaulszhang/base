// CatalogReg.cpp

#include "util/Util.h"
#include "util/dataview/CatalogReg.h"
#include "util/dataview/DataNode.h"
#include "util/dataview/DataViewError.h"
using namespace util::dataview::error;

#include <framework/string/StringToken.h>

using namespace boost::system;

namespace util
{

    namespace dataview
    {

        error_code CatalogReg::get_children_num(
            Path const & path, 
            size_t & n)
        {
            if (path.empty()) {
                n = children_.size();
                return error_succeed;
            } else {
                for (size_t i = 0; i < children_.size(); ++i) {
                    if (children_[i]->name == path[0]) {
                        if (children_[i]->type == t_cat) {
                            return ((CatalogNode *)children_[i])->get_children_num(path.sub_path(), n);
                        } else {
                            return error_invalid_param;
                        }
                    }
                }
                return error_no_such_item;
            }
        }

        error_code CatalogReg::get_children(
            Path const & path, 
            std::vector<Node> & children)
        {
            if (path.empty()) {
                for (size_t i = 0; i < children_.size(); ++i) {
                    children.push_back(*children_[i]);
                }
                return error_succeed;
            } else {
                for (size_t i = 0; i < children_.size(); ++i) {
                    if (children_[i]->name == path[0]) {
                        if (children_[i]->type == t_cat) {
                            return ((CatalogNode *)children_[i])->get_children(path.sub_path(), children);
                        } else {
                            return error_invalid_param;
                        }
                    }
                }
                return error_no_such_item;
            }
        }

        error_code CatalogReg::get_child_data_type(
            Path const & path, 
            DataType & data_type)
        {
            if (path.empty()) {
                return error_invalid_param;
            } else {
                for (size_t i = 0; i < children_.size(); ++i) {
                    if (children_[i]->name == path[0]) {
                        if (path.size() == 1) {
                            if (children_[i]->type == t_data) {
                                ((DataNode *)children_[i])->get_data_type(data_type);
                                return error_succeed;
                            }
                        } else {
                            if (children_[i]->type == t_cat) {
                                return ((CatalogNode *)children_[i])->get_child_data_type(path.sub_path(), data_type);
                            }
                        }
                        return error_invalid_param;
                    }
                }
                return error_no_such_item;
            }
        }

        error_code CatalogReg::get_child_data(
            Path const & path, 
            Data & data)
        {
            if (path.empty()) {
                return error_invalid_param;
            } else {
                for (size_t i = 0; i < children_.size(); ++i) {
                    if (children_[i]->name == path[0]) {
                        if (path.size() == 1) {
                            if (children_[i]->type == t_data) {
                                ((DataNode *)children_[i])->get_data(data);
                                return error_succeed;
                            }
                        } else {
                            if (children_[i]->type == t_cat) {
                                return ((CatalogNode *)children_[i])->get_child_data(path.sub_path(), data);
                            }
                        }
                        return error_invalid_param;
                    }
                }
                return error_no_such_item;
            }
        }

        error_code CatalogReg::get_table_head(
            Path const & path, 
            TableHead & table_head)
        {
            if (path.empty()) {
                return error_unsupported;
            } else {
                for (size_t i = 0; i < children_.size(); ++i) {
                    if (children_[i]->name == path[0]) {
                        if (children_[i]->type == t_cat) {
                            return ((CatalogNode *)children_[i])->get_table_head(path.sub_path(), table_head);
                        } else {
                            return error_invalid_param;
                        }
                    }
                }
                return error_no_such_item;
            }
        }

        error_code CatalogReg::get_table_data(
            Path const & path, 
            size_t from, 
            size_t size, 
            TableData & table)
        {
            if (path.empty()) {
                return error_unsupported;
            } else {
                for (size_t i = 0; i < children_.size(); ++i) {
                    if (children_[i]->name == path[0]) {
                        if (children_[i]->type == t_cat) {
                            return ((CatalogNode *)children_[i])->get_table_data(path.sub_path(), from, size, table);
                        } else {
                            return error_invalid_param;
                        }
                    }
                }
                return error_no_such_item;
            }
        }


        error_code CatalogReg::add_child(
            Node * node)
        {
            for (size_t i = 0; i < children_.size(); ++i) {
                if (children_[i]->name == node->name) {
                    return error_item_already_exist;
                }
            }
            children_.push_back(node);
            return error_succeed;
        }

        void CatalogReg::clear()
        {
            for (size_t i = 0; i < children_.size(); ++i) {
                delete children_[i];
            }
            children_.clear();
        }

    }

}
