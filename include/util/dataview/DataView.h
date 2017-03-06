// DataView.h

#ifndef _UTIL_DATAVIEW_DATA_VIEW_H_
#define _UTIL_DATAVIEW_DATA_VIEW_H_

//#include "util/dataview/NodeWrap.h"

namespace util
{

    namespace dataview
    {
        struct CatalogNode;
        struct Node;

        boost::system::error_code start(
            boost::asio::io_service & io_service);

        boost::system::error_code stop();

        CatalogNode & root();

        //template <typename T>
        //boost::system::error_code reg_node(
        //    std::string const & name, 
        //    T const & t)
        //{
        //    reg_node(new typename NodeWrap<T>::node_type(name, t));
        //}

        boost::system::error_code reg_node(
            Node * node);

    }
}

#endif // _UTIL_DATAVIEW_DATA_VIEW_H_
