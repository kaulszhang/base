// RtmpAmfType.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_AMF_TYPE_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_AMF_TYPE_H_

namespace util
{
    namespace protocol
    {

        struct RtmpAmfType // AMF Data Type
        {
            enum Enum {
                NUMBER      = 0x00,
                BOOL        = 0x01,
                STRING      = 0x02,
                OBJECT      = 0x03,
                MOVIECLIP   = 0x04,
                _NULL       = 0x05,
                UNDEFINED   = 0x06,
                REFERENCE   = 0x07,
                MIXEDARRAY  = 0x08,
                OBJECT_END  = 0x09,
                ARRAY       = 0x0a,
                DATE        = 0x0b,
                LONG_STRING = 0x0c,
                UNSUPPORTED = 0x0d,
            };
        };

        struct RtmpAmfString
        {
            enum {TYPE = RtmpAmfType::STRING};

            boost::uint16_t StringLength;
            std::string StringData;

            RtmpAmfString()
                : StringLength(0)
            {
            }

            RtmpAmfString(
                char const * str)
                : StringLength(strlen(str))
                , StringData(str)
            {
            }

            RtmpAmfString(
                std::string const & str)
                : StringLength(str.size())
                , StringData(str)
            {
            }

            friend bool operator==(
                RtmpAmfString const & l, 
                RtmpAmfString const & r)
            {
                return l.StringData == r.StringData;
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & StringLength;
                util::serialization::serialize_collection(ar, StringData, StringLength);
            }
        };

        struct RtmpAmfDate
        {
            enum {TYPE = RtmpAmfType::DATE};

            double DateTime;
            boost::int16_t LocalDateTimeOffset;

            friend bool operator==(
                RtmpAmfDate const & l, 
                RtmpAmfDate const & r)
            {
                return l.DateTime == r.DateTime 
                    && l.LocalDateTimeOffset == r.LocalDateTimeOffset;
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & DateTime;
                ar & LocalDateTimeOffset;
            }
        };

        struct RtmpAmfLongString
        {
            enum {TYPE = RtmpAmfType::LONG_STRING};

            boost::uint32_t StringLength;
            std::vector<boost::uint8_t> StringData;

            RtmpAmfLongString()
                : StringLength(0)
            {
            }

            RtmpAmfLongString(
                std::vector<boost::uint8_t> const & data)
                : StringLength(data.size())
                , StringData(data)
            {
            }

            friend bool operator==(
                RtmpAmfLongString const & l, 
                RtmpAmfLongString const & r)
            {
                return l.StringLength == r.StringLength
                    && l.StringData == r.StringData;
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & StringLength;
                util::serialization::serialize_collection(ar, StringData, StringLength);
            }
        };

        struct RtmpAmfObjectProperty;
        struct RtmpAmfValue;

        static inline bool FLV_Property_End(
            RtmpAmfObjectProperty const & Property);

        struct RtmpAmfObject
        {
            enum {TYPE = RtmpAmfType::OBJECT};

            std::vector<RtmpAmfObjectProperty> ObjectProperties;

            RtmpAmfValue & operator[](
                std::string const & key);

            RtmpAmfValue const & operator[](
                std::string const & key) const;

            friend bool operator==(
                RtmpAmfObject const & l, 
                RtmpAmfObject const & r)
            {
                return l.ObjectProperties == r.ObjectProperties;
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void load(
                Archive & ar);


            template <typename Archive>
            void save(
                Archive & ar) const;
        };

        struct RtmpAmfECMAArray
        {
            enum {TYPE = RtmpAmfType::MIXEDARRAY};

            boost::uint32_t ECMAArrayLength;
            std::vector<RtmpAmfObjectProperty> Variables;

            RtmpAmfValue & operator[](
                std::string const & key);

            RtmpAmfValue const & operator[](
                std::string const & key) const;

            friend bool operator==(
                RtmpAmfECMAArray const & l, 
                RtmpAmfECMAArray const & r)
            {
                return l.ECMAArrayLength == r.ECMAArrayLength
                    && l.Variables == r.Variables;
            }

            SERIALIZATION_SPLIT_MEMBER();

            template <typename Archive>
            void load(
                Archive & ar);
 

            template <typename Archive>
            void save(
                Archive & ar) const;
        };

        struct RtmpAmfValue;

        struct RtmpAmfStrictArray
        {
            enum {TYPE = RtmpAmfType::ARRAY};

            boost::uint32_t StrictArrayLength;
            std::vector<RtmpAmfValue> StrictArrayValue;

            friend bool operator==(
                RtmpAmfStrictArray const & l, 
                RtmpAmfStrictArray const & r)
            {
                return l.StrictArrayLength == r.StrictArrayLength
                    && l.StrictArrayValue == r.StrictArrayValue;
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & StrictArrayLength;
                util::serialization::serialize_collection(ar, StrictArrayValue, StrictArrayLength);
            }
        };

        struct RtmpAmfValue
        {
            enum {union_size = 48};

            boost::uint8_t Type;
            union {
                double Double;
                boost::uint64_t _Double;
                boost::uint8_t Bool;
                boost::uint16_t MovieClip;
                boost::uint16_t Null;
                boost::uint16_t Undefined;
                boost::uint16_t Reference;
                boost::uint16_t ObjectEndMarker;
                boost::uint8_t _union[union_size];
            };

            BOOST_STATIC_ASSERT(sizeof(RtmpAmfString) <= union_size);
            BOOST_STATIC_ASSERT(sizeof(RtmpAmfLongString) <= union_size);
            BOOST_STATIC_ASSERT(sizeof(RtmpAmfObject) <= union_size);
            BOOST_STATIC_ASSERT(sizeof(RtmpAmfECMAArray) <= union_size);
            BOOST_STATIC_ASSERT(sizeof(RtmpAmfECMAArray) <= union_size);

            RtmpAmfValue(
                RtmpAmfType::Enum type = RtmpAmfType::UNDEFINED)
                : Type(RtmpAmfType::UNDEFINED)
            {
                reset(type);
            }

            RtmpAmfValue(
                double num)
                : Type(RtmpAmfType::NUMBER)
                , Double(num)
            {
            }

            //RtmpAmfValue(
            //    bool b)
            //    : Type(RtmpAmfType::BOOL)
            //    , Bool(b)
            //{
            //}

            RtmpAmfValue(
                char const * str)
                : Type(RtmpAmfType::STRING)
            {
                construct<RtmpAmfString>(str);
            }

            RtmpAmfValue(
                std::string const & str)
                : Type(RtmpAmfType::STRING)
            {
                construct<RtmpAmfString>(str);
            }

            RtmpAmfValue(
                std::vector<boost::uint8_t> const & data)
                : Type(RtmpAmfType::LONG_STRING)
            {
                construct<RtmpAmfLongString>(data);
            }

            RtmpAmfValue(
                RtmpAmfValue const & r)
                : Type(RtmpAmfType::UNDEFINED)
            {
                (*this) = r;
            }

            ~RtmpAmfValue()
            {
                reset();
            }

            RtmpAmfValue & operator=(
                RtmpAmfValue const & r)
            {
                reset();
                Type = r.Type;
                switch (Type) {
                    case RtmpAmfType::STRING:
                        copy<RtmpAmfString>(r);
                        break;
                    case RtmpAmfType::OBJECT:
                        copy<RtmpAmfObject>(r);
                        break;
                    case RtmpAmfType::MIXEDARRAY:
                        copy<RtmpAmfECMAArray>(r);
                        break;
                    case RtmpAmfType::ARRAY:
                        copy<RtmpAmfStrictArray>(r);
                        break;
                    case RtmpAmfType::DATE:
                        copy<RtmpAmfDate>(r);
                        break;
                    case RtmpAmfType::LONG_STRING:
                        copy<RtmpAmfLongString>(r);
                        break;
                    default:
                        _Double = r._Double;
                        break;
                }
                return *this;
            }

            friend bool operator==(
                RtmpAmfValue const & l, 
                RtmpAmfValue const & r)
            {
                if (l.Type != r.Type) {
                    return false;
                }
                switch (l.Type) {
                    case RtmpAmfType::STRING:
                        return l.equal<RtmpAmfString>(r);
                    case RtmpAmfType::OBJECT:
                        return l.equal<RtmpAmfObject>(r);
                    case RtmpAmfType::MIXEDARRAY:
                        return l.equal<RtmpAmfECMAArray>(r);
                    case RtmpAmfType::ARRAY:
                        return l.equal<RtmpAmfStrictArray>(r);
                    case RtmpAmfType::DATE:
                        return l.equal<RtmpAmfDate>(r);
                    case RtmpAmfType::LONG_STRING:
                        return l.equal<RtmpAmfLongString>(r);
                    default:
                        return l._Double == r._Double;
                }
            }

        private:
            template <
                typename T
            >
            void construct()
            {
                assert(sizeof(T) <= sizeof(_union));
                new (_union)T;
            }

            template <
                typename T, 
                typename Arg
            >
            void construct(
                Arg & arg)
            {
                assert(sizeof(T) <= sizeof(_union));
                new (_union) T(arg);
            }

            template <
                typename T
            >
            void destroy()
            {
                ((T *)_union)->~T();
            }

            template <
                typename T
            >
            void copy(
            RtmpAmfValue const & r)
            {
                construct<T>(r.as<T>());
            }

            template <
                typename T
            >
            bool equal(
                RtmpAmfValue const & r) const
            {
                return as<T>() == r.as<T>();
            }

        public:
            template <
                typename T
            >
            T & as()
            {
                assert(Type == T::TYPE);
                return *(T *)_union;
            }

            template <
                typename T
            >
            T const & as() const
            {
                assert(Type == T::TYPE);
                return *(T const *)_union;
            }

            void reset()
            {
                switch (Type) {
                    case RtmpAmfType::STRING:
                        destroy<RtmpAmfString>();
                        break;
                    case RtmpAmfType::OBJECT:
                        destroy<RtmpAmfObject>();
                        break;
                    case RtmpAmfType::MIXEDARRAY:
                        destroy<RtmpAmfECMAArray>();
                        break;
                    case RtmpAmfType::ARRAY:
                        destroy<RtmpAmfStrictArray>();
                        break;
                    case RtmpAmfType::DATE:
                        destroy<RtmpAmfDate>();
                        break;
                    case RtmpAmfType::LONG_STRING:
                        destroy<RtmpAmfLongString>();
                        break;
                    default:
                        break;
                }
                Type = RtmpAmfType::UNDEFINED;
            }

            void reset(
                boost::uint8_t t)
            {
                if (t == Type) {
                    return;
                }
                reset();
                Type = t;
                switch (Type) {
                    case RtmpAmfType::STRING:
                        construct<RtmpAmfString>();
                        break;
                    case RtmpAmfType::OBJECT:
                        construct<RtmpAmfObject>();
                        break;
                    case RtmpAmfType::MIXEDARRAY:
                        construct<RtmpAmfECMAArray>();
                        break;
                    case RtmpAmfType::ARRAY:
                        construct<RtmpAmfStrictArray>();
                        break;
                    case RtmpAmfType::DATE:
                        construct<RtmpAmfDate>();
                        break;
                    case RtmpAmfType::LONG_STRING:
                        construct<RtmpAmfLongString>();
                        break;
                    default:
                        _Double = 0;
                        break;
                }
            }

            template <
                typename T
            >
            T & get()
            {
                if (T::TYPE != Type) {
                    reset();
                    Type = T::TYPE;
                    construct<T>();
                }
                return as<T>();
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                boost::uint8_t t = Type;
                ar & t;
                reset(t);
                switch (Type) {
                    case RtmpAmfType::NUMBER:
                        ar & _Double;
                        break;
                    case RtmpAmfType::BOOL:
                        ar & Bool;
                        break;
                    case RtmpAmfType::STRING:
                        ar & as<RtmpAmfString>();
                        break;
                    case RtmpAmfType::OBJECT:
                        ar & as<RtmpAmfObject>();
                        break;
                    case RtmpAmfType::MOVIECLIP:
                        ar.fail();
                        break;
                    case RtmpAmfType::_NULL:
                        break;
                    case RtmpAmfType::UNDEFINED:
                        break;
                    case RtmpAmfType::REFERENCE:
                        ar & Reference;
                        break;
                    case RtmpAmfType::MIXEDARRAY:
                        ar & as<RtmpAmfECMAArray>();
                        break;
                    case RtmpAmfType::OBJECT_END:
                        break;
                    case RtmpAmfType::ARRAY:
                        ar & as<RtmpAmfStrictArray>();
                        break;
                    case RtmpAmfType::DATE:
                        ar & as<RtmpAmfDate>();
                        break;
                    case RtmpAmfType::LONG_STRING:
                        ar & as<RtmpAmfLongString>();
                        break;
                    default:
                        ar.fail();
                        break;
                }
            }
        };

        struct RtmpAmfObjectProperty
        {
            RtmpAmfString PropertyName;
            RtmpAmfValue PropertyData;

            RtmpAmfObjectProperty() 
                : PropertyData(RtmpAmfType::OBJECT_END)
            {
            }

            template <typename T>
            RtmpAmfObjectProperty(
                std::string const & name, 
                T const & value)
                : PropertyName(name)
                , PropertyData(value)
            {
            }

            friend bool operator==(
                RtmpAmfObjectProperty const & l, 
                RtmpAmfObjectProperty const & r)
            {
                return l.PropertyName == r.PropertyName
                    && l.PropertyData == r.PropertyData;
            }

            template <typename Archive>
            void serialize(
                Archive & ar)
            {
                ar & PropertyName
                    & PropertyData;
            }
        };

        template <typename Archive>
        void RtmpAmfObject::load(
            Archive & ar)
        {
            RtmpAmfObjectProperty Property;
            ObjectProperties.clear();
            while (ar & Property) {
                ObjectProperties.push_back(Property);
                if (FLV_Property_End(Property))
                    break;
            }
        }

        template <typename Archive>
        void RtmpAmfObject::save(
            Archive & ar) const
        {
            for (size_t i = 0; i < ObjectProperties.size(); ++i) {
                ar & ObjectProperties[i];
            }
            ar & RtmpAmfObjectProperty();
        }


        template <typename Archive>
        void RtmpAmfECMAArray::load(
            Archive & ar)
        {
            ar & ECMAArrayLength;
            RtmpAmfObjectProperty Property;
            Variables.clear();
            while (ar & Property) {
                Variables.push_back(Property);
                if (FLV_Property_End(Property))
                    break;
            }
        }

        template <typename Archive>
        void RtmpAmfECMAArray::save(
            Archive & ar) const
        {
            boost::uint32_t ECMAArrayLength = Variables.size();
            ar & ECMAArrayLength;
            for (size_t i = 0; i < Variables.size(); ++i) {
                ar & Variables[i];
            }
            ar & RtmpAmfObjectProperty();
        }


        inline RtmpAmfValue & RtmpAmfObject::operator[](
            std::string const & key)
        {
            for (size_t i = 0; i < ObjectProperties.size(); ++i) {
                if (ObjectProperties[i].PropertyName.StringData == key) {
                    return ObjectProperties[i].PropertyData;
                }
            }
            ObjectProperties.push_back(RtmpAmfObjectProperty(key, RtmpAmfValue()));
            return ObjectProperties.back().PropertyData;
        }

        inline RtmpAmfValue const & RtmpAmfObject::operator[](
            std::string const & key) const
        {
            for (size_t i = 0; i < ObjectProperties.size(); ++i) {
                if (ObjectProperties[i].PropertyName.StringData == key) {
                    return ObjectProperties[i].PropertyData;
                }
            }
            assert(false);
            static RtmpAmfValue v;
            return v;
        }

        inline RtmpAmfValue & RtmpAmfECMAArray::operator[](
            std::string const & key)
        {
            for (size_t i = 0; i < Variables.size(); ++i) {
                if (Variables[i].PropertyName.StringData == key) {
                    return Variables[i].PropertyData;
                }
            }
            Variables.push_back(RtmpAmfObjectProperty(key, RtmpAmfValue()));
            return Variables.back().PropertyData;
        }

        inline RtmpAmfValue const & RtmpAmfECMAArray::operator[](
            std::string const & key) const
        {
            for (size_t i = 0; i < Variables.size(); ++i) {
                if (Variables[i].PropertyName.StringData == key) {
                    return Variables[i].PropertyData;
                }
            }
            assert(false);
            static RtmpAmfValue v;
            return v;
        }

        static inline bool FLV_Property_End(
            RtmpAmfObjectProperty const & Property)
        {
            return Property.PropertyData.Type == RtmpAmfType::OBJECT_END;
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_AMF_TYPE_H_
