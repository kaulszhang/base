// DataNode.h

#ifndef _UTIL_DATAVIEW_DATA_NODE_H_
#define _UTIL_DATAVIEW_DATA_NODE_H_

#include "util/dataview/Data.h"
#include "util/dataview/Node.h"

namespace util
{

    namespace dataview
    {

        struct DataNode
            : Node
        {
            DataNode(
                std::string const & name)
                : Node(name, node_type_enum)
            {
            }

            virtual ~DataNode()
            {
            }

            virtual void get_data_type(
                DataType & data_type) = 0;

            virtual void get_data(
                Data & data) = 0;

            static TypeEnum const node_type_enum = Node::t_data;
        };

    }
}

#endif // _UTIL_DATAVIEW_DATA_NODE_H_
