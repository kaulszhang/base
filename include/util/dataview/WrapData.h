// WrapData.h

#ifndef _UTIL_DATAVIEW_WRAPDATA_H_
#define _UTIL_DATAVIEW_WRAPDATA_H_

namespace util
{

    namespace dataview
    {

        template <typename C, typename F, F C::* M>
        struct Member
        {
            F & operator () (C & c)
            {
                return c.*M;
            }

            F const & operator () (C const & c)
            {
                return c.*M;
            }
        };

        template <typename F>
        struct DataTypeEnum
        {
            typedef F data_type;
            static DataType::TypeEnum const data_type_enum = DataType::t_nodata;
        };

        template <>
        struct DataTypeEnum<bool>
        {
            typedef bool data_type;
            static DataType::TypeEnum const data_type_enum = DataType::t_bool;
            typedef Member<Data, data_type, &Data::v_bool> member;
            //static data_type const Data::* const member = &Data::v_bool;
            typedef Member<ColumnData, std::vector<data_type>, &ColumnData::vec_bool> column_member;
            //static std::vector<data_type> const ColumnData::* const column_member = &ColumnData::vec_bool;
        };

        template <>
        struct DataTypeEnum<int>
        {
            typedef int data_type;
            static DataType::TypeEnum const data_type_enum = DataType::t_int;
            typedef Member<Data, data_type, &Data::v_int> member;
            //static data_type const Data::* const member = &Data::v_int;
            typedef Member<ColumnData, std::vector<data_type>, &ColumnData::vec_int> column_member;
            //static std::vector<data_type> const ColumnData::* const column_member = &ColumnData::vec_int;
        };

        template <>
        struct DataTypeEnum<float>
        {
            typedef float data_type;
            static DataType::TypeEnum const data_type_enum = DataType::t_float;
            typedef Member<Data, data_type, &Data::v_float> member;
            //static data_type const Data::* const member = &Data::v_float;
            typedef Member<ColumnData, std::vector<data_type>, &ColumnData::vec_float> column_member;
            //static std::vector<data_type> const ColumnData::* const column_member = &ColumnData::vec_float;
        };

        template <>
        struct DataTypeEnum<text>
        {
            typedef text data_type;
            static DataType::TypeEnum const data_type_enum = DataType::t_text;
            typedef Member<Data, data_type, &Data::v_text> member;
            //static data_type const Data::* const member = &Data::v_text;
            typedef Member<ColumnData, std::vector<data_type>, &ColumnData::vec_text> column_member;
            //static std::vector<data_type> const ColumnData::* const column_member = &ColumnData::vec_text;
        };

        template <typename F>
        struct DataT
            : Data
        {
            typedef F data_type;
            static DataType::TypeEnum const data_type_enum = DataTypeEnum<F>::data_type_enum;
            typedef typename DataTypeEnum<F>::member member;

            DataT()
                : Data(data_type_enum)
            {
            }

            DataT(
                F const & t)
                : Data(data_type_enum)
            {
                member()(*this) = t;
            }

            template <typename T>
            DataT(
                T const & t)
                : Data(data_type_enum)
            {
                member()(*this) = static_cast<F>(t);
            }
        };

        template <typename F>
        struct ColumnDataT
            : ColumnData
        {
            typedef F data_type;
            static DataType::TypeEnum const data_type_enum = DataTypeEnum<F>::data_type_enum;
            typedef typename DataTypeEnum<F>::member member;
            typedef typename DataTypeEnum<F>::column_member column_member;

            ColumnDataT()
                : ColumnData(data_type_enum)
            {
            }

            ColumnDataT(
                DataT<F> const & d) // column with one item
                : ColumnData(data_type_enum)
            {
                push(d);
            }

            void push(
                DataT<F> const & d)
            {
                std::vector<data_type> & vec = column_member()(*this);
                vec.push_back(member()(d));
            }

            void append(
                ColumnDataT<F> const & c)
            {
                std::vector<data_type> & vec = column_member()(*this);
                std::vector<data_type> & vec2 = column_member()(c);
                vec.insert(vec.begin(), vec2.begin(), vec2.end());
            }
        };


        template <typename T, typename F>
        struct DataNodeT
            : DataNode
        {
            typedef F data_type;
            static DataType::TypeEnum const data_type_enum = DataTypeEnum<F>::data_type_enum;
            typedef typename DataTypeEnum<F>::member member;

            DataNodeT(
                std::string const & name, 
                T const & t)
                : DataNode(name)
                , t_(t)
            {
            }

            virtual void get_data_type(
                DataType & data_type)
            {
                data_type = data_type_enum;
            }

            virtual void get_data(
                Data & data)
            {
                member()(data) = static_cast<data_type>(t_);
            }

            DataType get_data_type()
            {
                return data_type_enum;
            }

            DataT<F> get_data()
            {
                return DataT<F>(t_);
            }

        private:
            T const & t_;
        };

        template <typename T, typename F>
        struct WrapData
        {
            typedef DataNodeT<T, F> node_type;
        };

    }
}

#endif // _UTIL_DATAVIEW_WRAPDATA_H_
