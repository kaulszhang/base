// HttpHead.cpp

#include "util/Util.h"
#include "util/protocol/http/HttpHead.h"
#include "util/protocol/http/HttpError.h"
#include "util/protocol/http/HttpArchive.h"

using namespace boost::system;

namespace util
{
    namespace protocol
    {

        template <typename Archive>
        void serialize(
            Archive & ar, 
            util::protocol::HttpHead & t)
        {
            using namespace util::serialization;
            ar 
                & make_nvp("Host", t.host)
                & make_nvp("Content-Length", t.content_length)
                & make_nvp("Location", t.location)
                & make_nvp("Range", t.range)
                & make_nvp("Content-Range", t.content_range)
                & make_nvp("Connection", t.connection)
                & make_nvp("Pragma", t.pragma)
                ;
        }


        void HttpHead::add_fields(
            fields_map const & fields)
        {
            if (!fields.empty()) {
                HttpHeadIArchive ia(fields);
                ia >> *this;
                ia.load_other(other_fields_);
            }
        }

        void HttpHead::set_field(
            std::string const & name, 
            std::string const & value)
        {
            HttpStringField field(name, value);
            set_field(field);
        }

        void HttpHead::delete_field(
            std::string const & name)
        {
            HttpStringField field(name);
            set_field(field);
        }

        void HttpHead::get_content(
            std::ostream & os) const
        {
            error_code ec;
            get_content(os, ec);
        }

        void HttpHead::get_content(
            std::ostream & os, 
            error_code & ec) const
        {
            std::string line;
            bool ret = get_line(line);
            (void)ret;
            assert(ret);
            os << line << "\r\n";
            {
                HttpHeadOArchive oa(os);
                oa << *this;
                oa.save_other(other_fields_);
                assert(oa);
            }
            os << "\r\n";
            ec = error_code();
        }

        void HttpHead::set_content(
            std::istream & is)
        {
            error_code ec;
            set_content(is, ec);
            assert(!ec);
        }

        void HttpHead::set_content(
            std::istream & is, 
            error_code & ec)
        {
            other_fields_.clear();
            std::string line;
            std::getline(is, line);
            assert(is);
            trim(line);
            bool ret = set_line(line);
            if (!ret) {
                ec = http_error::format_error;
                return;
            }
            HttpHeadIArchive ia(is);
            ia >> *this;
            if (!ia) {
                ec = http_error::format_error;
                return;
            }
            ia.load_other(other_fields_);
            assert(ia);
        }

        void HttpHead::check_handler(
            HttpStringField & field)
        {
            if (field.head_) {
                assert(this == field.head_);
            } else {
                field.handler_ = std::auto_ptr<HttpStringField::Handler>();
            }
            if (!field.handler_.get()) {
                HttpHeadVisitor visitor(field);
                visitor.visit(*this);
                visitor.visit_other(other_fields_);
                field.handler_ = visitor.handler();
            }
        }

        void HttpHead::set_field(
            HttpStringField const & field)
        {
            check_handler(const_cast<HttpStringField &>(field));
            field.set();
        }

        void HttpHead::get_field(
            HttpStringField & field) const
        {
            const_cast<HttpHead &>(*this).check_handler(field);
            field.get();
        }

    } // namespace protocol
} // namespace util
