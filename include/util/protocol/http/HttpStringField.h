// HttpStringField.h

#ifndef _UTIL_PROTOCOL_HTTP_STRING_FIELD_H_
#define _UTIL_PROTOCOL_HTTP_STRING_FIELD_H_

#include <memory>

namespace util
{
    namespace protocol
    {
        class HttpHead;

        class HttpStringField
        {
        public:
            HttpStringField(
                std::string const & name)
                : head_(NULL)
                , name_(name)
                , empty_(true)
            {
            }

            HttpStringField(
                std::string const & name, 
                std::string const & value)
                : head_(NULL)
                , name_(name)
                , empty_(false)
                , value_(value)
            {
            }

            std::string const & name() const
            {
                return name_;
            }

            std::string const & value() const
            {
                return value_;
            }

            operator std::string() const
            {
                if (empty_)
                    return std::string();
                else
                    return value_;
            }

            HttpStringField & operator=(
                std::string const & str)
            {
                empty_ = false;
                value_ = str;
                set();
                return *this;
            }

            void get()
            {
                handler_->get(*this);
            }

            void set() const
            {
                handler_->set(*this);
            }

        public:
            class Handler
            {
            public:
                virtual ~Handler() {};

            public:
                void set(
                    HttpStringField const & field)
                {
                    set(field.empty_ , field.value_);
                }

                void get(
                    HttpStringField & field)
                {
                    get(field.empty_ , field.value_);
                }

            private:
                virtual void set(
                    bool empty, 
                    std::string const & value) = 0;

                virtual void get(
                    bool & empty, 
                    std::string & value) const = 0;
            };

        private:
            friend class HttpHead;
            friend class Handler;

            HttpHead * head_;
            std::string name_;
            bool empty_;
            std::string value_;
            std::auto_ptr<Handler> handler_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_STRING_FIELD_H_
