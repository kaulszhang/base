// CatalogReg.h

#ifndef _UTIL_DATAVIEW_CATALOG_REG_H_
#define _UTIL_DATAVIEW_CATALOG_REG_H_

#include "util/dataview/CatalogNode.h"

namespace util
{

    namespace dataview
    {

        struct CatalogReg
            : CatalogNode
        {
            CatalogReg(
                std::string const & name = "")
                : CatalogNode(name)
            {
            }

            virtual boost::system::error_code get_children_num(
                Path const & path, 
                size_t & n);

            virtual boost::system::error_code get_children(
                Path const & path, 
                std::vector<Node> & children);

            virtual boost::system::error_code get_child_data_type(
                Path const & path, 
                DataType & data_type);

            virtual boost::system::error_code get_child_data(
                Path const & path, 
                Data & data);

            virtual boost::system::error_code get_table_head(
                Path const & path, 
                TableHead & table_head);

            virtual boost::system::error_code get_table_data(
                Path const & path, 
                size_t from, 
                size_t size, 
                TableData & data);

            boost::system::error_code add_child(
                Node * node);

            void clear();

        private:
            std::vector<Node *> children_;
        };

    }
}

#endif // _UTIL_DATAVIEW_CATALOG_REG_H_

