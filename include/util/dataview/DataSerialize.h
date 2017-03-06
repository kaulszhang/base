// DataSerialize.h

#ifndef _UTIL_DATAVIEW_DATA_SERIALIZE_H_
#define _UTIL_DATAVIEW_DATA_SERIALIZE_H_

#include "util/serialization/Serialization.h"
#include "util/serialization/stl/vector.h"

namespace util
{
    namespace dataview
    {
        // DataType
        template<typename Archive>
        void DataType::serialize(
            Archive & ar)
        {
            //ar & type;
        }

        // Data
        template<typename Archive>
        void Data::serialize(
            Archive & ar)
        {
            //ar & SERIALIZATION_NVP(type);
            switch (type.type) {
                case DataType::t_bool:
                    ar & SERIALIZATION_NVP(v_bool);
                    break;
                case DataType::t_int:
                    ar & SERIALIZATION_NVP(v_int);
                    break;
                case DataType::t_float:
                    ar & SERIALIZATION_NVP(v_float);
                    break;
                case DataType::t_text:
                    ar & SERIALIZATION_NVP(v_text);
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        // ColumnHead
        template<typename Archive>
        void ColumnHead::serialize(
            Archive & ar)
        {
           // ar & name 
           //     & type;
        }

        // TableHead
        template<typename Archive>
        void TableHead::serialize(
            Archive & ar)
        {
            //ar & columns 
            //    & item_type 
            //    & column_child 
            //    & item_count;
        }

        // ColumnData
        template<typename Archive>
        void ColumnData::serialize(Archive & ar)
        {
            //ar & SERIALIZATION_NVP(type);
            switch (type.type) {
            case DataType::t_bool:
                ar & SERIALIZATION_NVP(vec_bool);
                break;
            case DataType::t_int:
                ar & SERIALIZATION_NVP(vec_int);
                break;
            case DataType::t_float:
                ar & SERIALIZATION_NVP(vec_float);
                break;
            case DataType::t_text:
                ar & SERIALIZATION_NVP(vec_text);
                break;
            default:
                assert(0);
                break;
            }
        }

        // ItemHead
        template<typename Archive>
        void ItemHead::serialize(Archive & ar)
        {
            ar & SERIALIZATION_NVP(name);
        }

        // ItemData
        template<typename Archive>
        void ItemData::serialize(Archive & ar)
        {
            ar & SERIALIZATION_NVP(datas);
        }

        // TableData
        template<typename Archive>
        void TableData::serialize(Archive & ar)
        {
            //ar & SERIALIZATION_NVP(count)
            ar  & SERIALIZATION_NVP(items)
                & SERIALIZATION_NVP(datas);
        }

        // Node
        template<typename Archive>
        void Node::serialize(
            Archive & ar)
        {
            ar & SERIALIZATION_NVP(name);
            ar & SERIALIZATION_NVP(type);
        }

        // Path
        template<typename Archive>
        void Path::serialize(
            Archive & ar)
        {
            ar & *(std::vector<std::string> *)this;
        }

    }
}

#endif // _UTIL_DATAVIEW_DATA_SERIALIZE_H_
