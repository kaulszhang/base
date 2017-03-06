// Version.h

#ifndef _FRAMEWORK_SYSTEM_VERSION_H_
#define _FRAMEWORK_SYSTEM_VERSION_H_

namespace framework
{
    namespace system
    {

        class Version
        {
        public:
            Version()
                : value(0)
            {
            }

            Version(
                boost::uint8_t major, 
                boost::uint8_t minor, 
                boost::uint16_t revision = 0, 
                boost::uint32_t build = 0);

            Version(
                std::string const & str);

        public:
            boost::system::error_code from_string(
                std::string const & str);

            std::string to_string() const;

            std::string to_simple_string() const;

        public:
            boost::uint8_t major() const
            {
                return ((boost::uint8_t *)&value)[0];
            }

            boost::uint8_t minor() const
            {
                return ((boost::uint8_t *)&value)[1];
            }

            boost::uint16_t revision() const
            {
                return (((boost::uint8_t *)&value)[2] << 8)
                    | (((boost::uint8_t *)&value)[3]);
            }

            boost::uint32_t build() const
            {
                return (((boost::uint8_t *)&value)[4] << 24)
                    | (((boost::uint8_t *)&value)[5] << 16)
                    | (((boost::uint8_t *)&value)[6] << 8)
                    | (((boost::uint8_t *)&value)[7]);
            }

        public:
            static boost::system::error_code get_version(
                std::string const & file, 
                std::string const & module, 
                std::string & version);

            static boost::system::error_code get_version(
                std::string const & file, 
                std::map<std::string, std::string> & module_versions);

            static boost::system::error_code set_version(
                std::string const & file, 
                std::string const & module, 
                std::string const & version);

        private:
            static boost::system::error_code file_version(
                std::string const & file, 
                std::string const & module, 
                bool get_or_set, 
                std::string & version);

        private:
            boost::uint64_t value;
        };

    } // namespace system
} // namespace boost

#endif // _FRAMEWORK_SYSTEM_VERSION_H_
