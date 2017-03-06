// DataView.cpp

#include "util/Util.h"
#include "util/dataview/DataView.h"
#include "util/dataview/DataViewError.h"
#include "util/dataview/CatalogReg.h"
using namespace util::dataview::error;

using namespace boost::system;

namespace util
{

    namespace dataview
    {

        size_t log_level = 2;
        CatalogReg root_;

        error_code start(
            boost::asio::io_service & io_service)
        {
            return error_succeed;
        }

        error_code stop()
        {
            root_.clear();
            return error_succeed;
        }

        CatalogNode & root()
        {
            return root_;
        }

        error_code reg_node(
            Node * node)
        {
            return root_.add_child(node);
        }

    }
}
