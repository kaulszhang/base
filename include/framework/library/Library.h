// Library.h

#ifndef _FRAMEWORK_LIBRARY_LIBRARY_H_
#define _FRAMEWORK_LIBRARY_LIBRARY_H_

namespace framework
{
    namespace library
    {

        class Library
        {
        public:
            Library();

            Library(
                Library const & r);

            Library(
                std::string const & name);

            ~Library();

        public:
            boost::system::error_code open(
                std::string const & name);

            bool is_open() const
            {
                return handle_ != NULL;
            }

            boost::system::error_code close();

        public:
            void * symbol(
                std::string const & name) const;

            std::string path() const;

        public:
            static Library from_address(
                void * addr);

            static Library self();

        private:
            Library(
                void * handle);

        private:
            void * handle_;
            bool need_close_;
        };

    } // namespace library
} // namespace framework

#endif // _FRAMEWORK_LIBRARY_LIBRARY_H_
