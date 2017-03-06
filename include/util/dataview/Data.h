// Data.h

#ifndef _UTIL_DATAVIEW_DATA_H_
#define _UTIL_DATAVIEW_DATA_H_

#include <utility>

namespace util
{

    namespace dataview
    {

        typedef std::string text;

        struct DataType
        {
            enum TypeEnum
            {
                t_bool, 
                t_int, 
                t_float, 
                t_text, 
                t_nodata, 
            };

            DataType()
                : type(t_nodata)
            {
            }

            DataType(
                TypeEnum type)
                : type(type)
            {
            }

            void swap(
                DataType & r)
            {
                std::swap(type, r.type);
            }

            void clear()
            {
                type = DataType::t_nodata;
            }

            friend bool operator == (
                DataType const & l, 
                DataType const & r)
            {
                return l.type == r.type;
            }

            friend bool operator != (
                DataType const & l, 
                DataType const & r)
            {
                return l.type != r.type;
            }

            TypeEnum type;

            template<typename Archive>
            void serialize(Archive & ar);
        };

        struct Data
        {
            Data(
                DataType::TypeEnum type = DataType::t_nodata)
                : type(type)
            {
            }

            Data(
                DataType const & dt)
                : type(dt)
            {
            }

            Data(
                bool b)
                : type(DataType::t_bool)
                , v_bool(b)
            {
            }

            Data(
                int i)
                : type(DataType::t_int)
                , v_int(i)
            {
            }

            Data(
                size_t i)
                : type(DataType::t_int)
                , v_int((int)i)
            {
            }

            Data(
                float f)
                : type(DataType::t_float)
                , v_float(f)
            {
            }

            Data(
                double d)
                : type(DataType::t_float)
                , v_float((float)d)
            {
            }

            Data(
                text const & t)
                : type(DataType::t_text)
                , v_text(t)
            {
            }

            Data(
                Data const & d)
            {
                operator = (d);
            }

            Data & operator = (
                Data const & d)
            {
                type = d.type;
                switch (type.type) {
                    case DataType::t_bool:
                        v_bool = d.v_bool;
                        break;
                    case DataType::t_int:
                        v_int = d.v_int;
                        break;
                    case DataType::t_float:
                        v_float = d.v_float;
                        break;
                    case DataType::t_text:
                        v_text = d.v_text;
                        break;
                    default:
                        break;
                }
                return *this;
            }

            DataType type;

            bool v_bool;
            int v_int;
            float v_float;
            text v_text;

            void swap(
                Data & r)
            {
                type.swap(r.type);
                std::swap(v_bool, r.v_bool);
                std::swap(v_int, r.v_int);
                std::swap(v_float, r.v_float);
                std::swap(v_text, r.v_text);
            }

            void clear()
            {
                type = DataType::t_nodata;
                v_text.clear();
            }

            template<typename Archive>
            void serialize(Archive & ar);
        };
    }
}

#endif // _UTIL_DATAVIEW_DATA_H_
