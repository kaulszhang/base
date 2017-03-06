// PacketDataView.h

#ifndef _UTIL_DATAVIEW_PACKET_DATA_VIEW_H_
#define _UTIL_DATAVIEW_PACKET_DATA_VIEW_H_

#include "util/archive/JsonOArchive.h"
#include "util/serialization/stl/vector.h"

#include "util/dataview/DataView.h"
#include "util/dataview/DataSerialize.h"

#include <string>
#include <vector>

namespace util
{

    namespace dataview
    {
        typedef util::archive::JsonOArchive<> oarchive;

        struct packet_login
        {
            struct output_type
            {
                size_t uid;

                template<typename Archive>
                void serialize(Archive & ar)
                {
                    ar & SERIALIZATION_NVP(uid);
                }
            } output;
        };

        struct packet_get_children
        {
            struct output_type
            {
                std::vector<util::dataview::Node> children;

                template<typename Archive>
                void serialize(Archive & ar)
                {
                    ar & SERIALIZATION_NVP(children);
                }
            } output;
        };

        struct packet_get_table_head
        {
            struct output_type
            {
                util::dataview::TableHead table_head;

                template<typename Archive>
                void serialize(Archive & ar)
                {
                    ar & SERIALIZATION_NVP(table_head);
                }
            } output;
        };

        struct packet_get_table_data
        {
            struct input_type
            {
                size_t uid;
                util::dataview::Path path;
                size_t from;
                size_t size;

                template<typename Archive>
                void serialize(Archive & ar)
                {
                    ar & SERIALIZATION_NVP(uid)
                        & SERIALIZATION_NVP(path)
                        & SERIALIZATION_NVP(from)
                        & SERIALIZATION_NVP(size);
                }
            } input;

            struct output_type
            {
                util::dataview::TableData table_data;

                template<typename Archive>
                void serialize(Archive & ar)
                {
                    ar & SERIALIZATION_NVP(table_data);
                }
            } output;
        };

        struct packet_get_line_data
        {
            struct output_type
            {
                std::vector<util::dataview::Data> line_datas;

                template<typename Archive>
                void serialize(Archive & ar)
                {
                    ar & SERIALIZATION_NVP(line_datas);
                }
            } output;
        };

        struct packet_add_line
        {
            struct input_type
            {
                size_t uid;
                util::dataview::Path path;

                template<typename Archive>
                void serialize(Archive & ar)
                {
                    ar & SERIALIZATION_NVP(uid);
                    ar & SERIALIZATION_NVP(path);
                }
            } input;
        };

        struct packet_del_line
        {
            struct input_type
            {
                size_t uid;
                util::dataview::Path path;

                template<typename Archive>
                void serialize(Archive & ar)
                {
                    ar & SERIALIZATION_NVP(uid);
                    ar & SERIALIZATION_NVP(path);
                }
            } input;
        };

        struct packet_add_logs
        {
            struct  input_type
            {
                size_t uid;
                util::dataview::Path path;
                int interval;
                std::string fileName;
                int type;

                template<typename Archive>
                void serialize(Archive & ar)
                {
                    ar & SERIALIZATION_NVP(uid);
                    ar & SERIALIZATION_NVP(path);
                    ar & SERIALIZATION_NVP(interval);
                    ar & SERIALIZATION_NVP(fileName);
                    ar & SERIALIZATION_NVP(type);
                }
            } input;
        };
    }
}

#endif // _UTIL_DATAVIEW_PACKET_DATA_VIEW_H_
