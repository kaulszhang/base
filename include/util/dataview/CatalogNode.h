// CatalogNode.h

#ifndef _UTIL_DATAVIEW_CATALOG_NODE_H_
#define _UTIL_DATAVIEW_CATALOG_NODE_H_

#include "util/dataview/Data.h"
#include "util/dataview/Node.h"
#include "util/dataview/Table.h"

namespace util
{

    namespace dataview
    {

        struct CatalogNode
            : Node
        {
            static bool const single_line = false;
            static TypeEnum const node_type_enum = Node::t_cat;

            CatalogNode(
                std::string const & name)
                : Node(name, node_type_enum)
            {
            }

            virtual ~CatalogNode()
            {
            }

            virtual boost::system::error_code get_children_num(
                Path const & path, 
                size_t & n) = 0;

            virtual boost::system::error_code get_children(
                Path const & path, 
                std::vector<Node> & children) = 0;

            virtual boost::system::error_code get_child_data_type(
                Path const & path, 
                DataType & data_type) = 0;

            virtual boost::system::error_code get_child_data(
                Path const & path, 
                Data & data) = 0;

            virtual boost::system::error_code get_table_head(
                Path const & path, 
                TableHead & table_head);

            virtual boost::system::error_code get_table_data(
                Path const & path, 
                size_t from, 
                size_t size, 
                TableData & table_data);
        };

    }
}

#endif // _UTIL_DATAVIEW_CATALOG_NODE_H_
