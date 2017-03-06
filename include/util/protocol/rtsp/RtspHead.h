// RtspHead.h

#ifndef _UTIL_PROTOCOL_RTSP_HEAD_H_
#define _UTIL_PROTOCOL_RTSP_HEAD_H_

#include "util/protocol/rtsp/RtspStringField.h"
#include "util/protocol/rtsp/RtspFieldRange.h"
#include "util/protocol/rtsp/RtspFieldConnection.h"

#include <boost/optional.hpp>

#include <istream>
#include <ostream>

namespace util
{
    namespace protocol
    {

        class RtspHead
        {
        public:
            typedef std::map<std::string, std::vector<std::string> > fields_map;
            typedef fields_map::const_iterator fields_map_iterator;
            typedef fields_map::value_type field_type;

        public:
            RtspHead() {};
            
            virtual ~RtspHead() {}

        public:
            void add_fields(
                fields_map const & fields);

        public:
            void set_field(
                std::string const & name, 
                std::string const & value);

            void delete_field(
                std::string const & name);

            RtspStringField operator[](
                std::string const & name)
            {
                RtspStringField field(name);
                get_field(field);
                return field;
            }

        public:
            void get_content(
                std::ostream & os) const;

            void set_content(
                std::istream & is);

            void get_content(
                std::ostream & os, 
                boost::system::error_code & ec) const;

            void set_content(
                std::istream & is, 
                boost::system::error_code & ec);

        public:
            boost::optional<boost::uint64_t> content_length;
            boost::optional<std::string> host;
            boost::optional<std::string> location;
            boost::optional<std::string> public_;
            boost::optional<rtsp_field::Range> range;
            boost::optional<std::string> rtp_info;
            boost::optional<std::string> transport;
            std::vector<std::string> pragma;

        private:
            void check_handler(
                RtspStringField & field);

            void set_field(
                RtspStringField const & field);

            void get_field(
                RtspStringField & field) const;

        private:
            virtual bool get_line(
                std::string & line) const = 0;

            virtual bool set_line(
                std::string const & line) = 0;

        private:
            fields_map other_fields_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTSP_HEAD_H_
