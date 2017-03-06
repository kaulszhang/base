// Uuid.h

#ifndef _FRAMEWORK_STRING_GUID_H_
#define _FRAMEWORK_STRING_GUID_H_

#include <boost/static_assert.hpp>
#include <boost/array.hpp>

namespace framework
{
    namespace string
    {

		struct UUID
        {
            boost::uint32_t Data1;
            boost::uint16_t Data2;
            boost::uint16_t Data3;
            boost::uint8_t Data4[8];
        };

        BOOST_STATIC_ASSERT(sizeof(UUID) == 16);

        class Uuid
        {
        public:
            typedef boost::array<boost::uint8_t, sizeof(UUID)> bytes_type;

        public:
            static Uuid const & null()
            {
                return Null;
            }

        public:
            Uuid()
            {
                clear();
            }

            explicit Uuid(
                std::string const & str)
            {
                from_string(str);
            }

            explicit Uuid(
                bytes_type const & bytes)
            {
                from_bytes(bytes);
            }

            Uuid(
                UUID const & uuid)
            {
                assign(uuid);
            }

        public:
            std::string to_string() const;

            boost::system::error_code from_string(
                std::string const & str);

        public:
            bytes_type to_bytes() const;

            void from_bytes(
                bytes_type const & bytes);

            bytes_type to_little_endian_bytes() const;

            void from_little_endian_bytes(
                bytes_type const & bytes);

            bytes_type to_big_endian_bytes() const;

            void from_big_endian_bytes(
                bytes_type const & bytes);

        public:
            bool is_empty() const
            {
                return (*this) == Uuid::Null;
            }

            void assign(
                UUID const & uuid)
            {
                uuid_ = uuid;
            }

            UUID const & data() const
            {
                return uuid_;
            }

            std::size_t hash_value() const
            {
                return uuid_.Data1 
                    ^ ((uuid_.Data2 << 0x10) | uuid_.Data3) 
                    ^ ((uuid_.Data4[0] << 0x18) | (uuid_.Data4[1] << 0x10) | (uuid_.Data4[2] << 0x8) | uuid_.Data4[3]) 
                    ^ ((uuid_.Data4[4] << 0x18) | (uuid_.Data4[5] << 0x10) | (uuid_.Data4[6] << 0x8) | uuid_.Data4[7]);
            }
    
            void clear()
            {
                memset((char*)&uuid_, 0, sizeof(uuid_));
            }

            void generate();

        public:
            friend bool operator==(
                Uuid const & l, 
                Uuid const & r)
            {
                return memcmp(&l.uuid_, &r.uuid_, sizeof(l.uuid_)) == 0;
            }

            friend bool operator!=(
                Uuid const & l, 
                Uuid const & r)
            {
                return !(l == r);
            }

            friend bool operator<(
                Uuid const & l, 
                Uuid const & r)
            {
                return memcmp(&l.uuid_, &r.uuid_, sizeof(l.uuid_)) < 0;
            }

        public:
            template<
                class _Elem, 
                class _Traits
            >
            friend inline std::basic_ostream<_Elem, _Traits> & operator<<(
                std::basic_ostream<_Elem, _Traits> & os, const Uuid & uuid)
            {
                return os << uuid.to_string();
            }

        private:
            static Uuid const Null;

        private:
            UUID uuid_;
        };

    } // namespace string
} // namespace framework

namespace boost
{
    inline std::size_t hash_value(
        const framework::string::Uuid & uuid)
    {
        return uuid.hash_value();
    }
}

#endif // _FRAMEWORK_STRING_GUID_H_
