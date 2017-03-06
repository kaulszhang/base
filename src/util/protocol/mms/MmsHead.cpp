// MmsHead.cpp

#include "util/Util.h"
#include "util/protocol/mms/MmsHead.h"
#include "util/protocol/mms/MmsError.h"

#include <util/archive/BinaryIArchive.h>
#include <util/archive/BinaryOArchive.h>

using namespace util::archive;
using namespace boost::system;

namespace util
{
    namespace protocol
    {


        void MmsHead::get_content(
            std::ostream & os) const
        {
            error_code ec;
            get_content(os, ec);
        }

        void MmsHead::get_content(
            std::ostream & os, 
            error_code & ec) const
        {
            util::archive::BinaryOArchive<> oa( os );
            oa << *this;

            ec = error_code();
        }

        void MmsHead::set_content(
            std::istream & is)
        {
            error_code ec;
            set_content(is, ec);
            assert(!ec);
        }

        void MmsHead::set_content(
            std::istream & is, 
            error_code & ec)
        {
            util::archive::BinaryIArchive<> ia( is );
            ia >> *this;
            content_length = trunk_len * 8 - 8;

            ec = error_code();
        }

    } // namespace protocol
} // namespace util
