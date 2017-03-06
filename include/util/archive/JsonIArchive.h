// // JsonIArchive.h

#ifndef _UTIL_ARCHIVE_JSON_I_ARCHIVE_H_
#define _UTIL_ARCHIVE_JSON_I_ARCHIVE_H_

#include "util/archive/StreamIArchive.h"
#include "util/serialization/Optional.h"

#include <framework/string/Parse.h>

#include <iterator>
#include <json/json.h>

namespace util
{
    namespace archive
    {
        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
		>
		class JsonIArchive
			: public StreamIArchive<JsonIArchive<_Elem, _Traits>, _Elem, _Traits>
		{
			friend class StreamIArchive<JsonIArchive<_Elem, _Traits>, _Elem, _Traits>;
		public:
			JsonIArchive(
				std::basic_istream<_Elem, _Traits> & is)
				: StreamIArchive<JsonIArchive<_Elem, _Traits>, _Elem, _Traits>(*is.rdbuf())
			{
				load_json();
			}

			JsonIArchive(
				std::basic_streambuf<_Elem, _Traits> & buf)
				: StreamIArchive<JsonIArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
			{
				load_json();
			}

		public:
			/// 从流中读出变量
			template<class T>
			void load(
				T & t)
			{
				Value & v = value_stack_.back();
				char const * value = NULL;
				if(v.type == Value::t_jsonType)
				{
					switch (v.value.type()) 
					{
					case Json::intValue:
                        t = v.value.asInt64();
						return;
					case Json::uintValue:
                        t = v.value.asUInt64();
						return;
					case Json::realValue:
						t = v.value.asDouble();
						return;
					case Json::stringValue:
						value = v.value.asString().c_str();
						break;
					case Json::booleanValue:
						t = v.value.asBool();
						return;
					case Json::arrayValue:
						assert(0);
						break;
					case Json::objectValue:
						assert(0);
						break;
					default:
						assert(false);
					}
					if (value == NULL || framework::string::parse2(value, t)) {
						this->state(2);
					} 
				}
				else if(v.type == Value::t_count)
				{
					t = v.count;
				}
			}

            /// 从流中读出标准库字符串
            void load(
                std::string & t)
            {
				Value & vp = value_stack_.back();
				t = vp.value.asString();
            }

            /// 从流中读出可选值
            template<class T>
            void load(
                boost::optional<T> & t)
            {
             
            }

            /// 从流中读出可选值
            template<class T>
            void load(
                util::serialization::optional_ref<T> & t)
            {
              
            }

            using StreamIArchive<JsonIArchive<_Elem, _Traits>, _Elem, _Traits>::load;

            void load_start(
                std::string const & name)
            {
				Value & vp = value_stack_.back();
				Value v;
				if(vp.value.type() == Json::arrayValue)
				{
					if(name == "count")
					{
						v.type = Value::t_count;
						v.count = vp.value.size();
					}
					else if(name == "item")
					{
                        assert(vp.value.size() > vp.index);
                        if (vp.index >= vp.value.size())
                        {
                            this->state(1);
                            return;
                        }
						v.value = vp.value[(int)vp.index++];
						v.type = Value::t_jsonType;
					}
				}
				else
				{
					v.type = Value::t_jsonType;
                    v.value = vp.value[name];
                    if (!vp.value.isMember(name))
                    {
                        this->state(1);
                    }
				}
				value_stack_.push_back(v);
            }

            void load_end(
                std::string const & name)
            {
				int t = value_stack_.back().type;
				value_stack_.pop_back();
            }

        private:
            void load_json()
            {
				std::string str;
                std::basic_istream<_Elem, _Traits> is(this->buf_);
                std::istream_iterator<char, _Elem, _Traits> beg(is);
                std::istream_iterator<char, _Elem, _Traits> end;
                is >> std::noskipws;
                std::copy(beg, end, std::back_inserter(str));
                if(!reader_.parse(str.c_str(), root_))
                    this->state(1);
				else
					value_stack_.push_back(Value(root_));
            }

        private:
			struct Value
            {
                Value(Json::Value& v)
                    : type(t_jsonType)
                    , value(v)
                {
					index = 0;
					count = 0;
                }

				Value()
                    : type(t_nullValue)
                {
					index = 0;
					count = 0;
                }

                enum TypeEnum
                {
					t_nullValue = 0, 
					t_jsonType, 
					t_count
                } type;

				Json::Value value;

				size_t index;
				size_t count;
            };


			std::vector<Value> value_stack_;
			Json::Reader reader_;
			Json::Value root_;
        };

    } // namespace archive
} // namespace util

namespace util
{
    namespace serialization
    {

        template<
            typename _Elem, 
            typename _Traits, 
            class _T
        >
        struct is_primitive<util::archive::JsonIArchive<_Elem, _Traits>, boost::optional<_T> >
            : boost::true_type
        {
        };

        template<
            class _T
        >
        struct optional_ref;

        template<
            typename _Elem, 
            typename _Traits, 
            class _T
        >
        struct is_primitive<util::archive::JsonIArchive<_Elem, _Traits>, optional_ref<_T> >
            : boost::true_type
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_JSON_I_ARCHIVE_H_
