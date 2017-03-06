// HttpArchive.h 

#ifndef _UTIL_PROTOCOL_HTTP_HTTP_ARCHIVE_H_
#define _UTIL_PROTOCOL_HTTP_HTTP_ARCHIVE_H_

#include "util/serialization/NVPair.h"
#include "util/serialization/stl/vector.h"
#include "util/archive/BasicIArchive.h"
#include "util/archive/BasicOArchive.h"
using namespace util::archive;

#include <framework/string/Algorithm.h>
#include <framework/string/Parse.h>
#include <framework/string/ParseStl.h>
#include <framework/string/Format.h>
#include <framework/string/FormatStl.h>
using namespace framework::string;

using namespace boost::system;

namespace util
{
    namespace protocol
    {
        class HttpHeadIArchive;
        class HttpHeadOArchive;
    }

    namespace serialization
    {

        template <typename T>
        struct is_primitive<util::protocol::HttpHeadIArchive, boost::optional<T> >
            : boost::true_type
        {
        };

        template <typename T>
        struct is_primitive<util::protocol::HttpHeadOArchive, boost::optional<T> >
            : boost::true_type
        {
        };

    }

    namespace protocol
    {

        class HttpHeadIArchive
            : public BasicIArchive<HttpHeadIArchive>
        {
            friend class BasicIArchive<HttpHeadIArchive>;
            friend struct LoadAccess;

            typedef std::map<std::string, std::vector<std::string> > fields_map;
            typedef fields_map::iterator fields_map_iterator;

        public:
            HttpHeadIArchive(
                std::istream & is, 
                bool clear_optional = true)
                : clear_optional_(clear_optional)
                , sub_level_(0)
                , sub_item_(0)
            {
                std::string field;
                while (std::getline(is, field) && field != "\r") {
                    trim(field);
                    if (field.empty())
                        break;
                    std::string::size_type p = field.find(':');
                    if (p != std::string::npos) {
                        std::string n = field.substr(0, p);
                        trim(n);
                        std::string v = field.substr(p + 1);
                        trim(v);
                        fields_[n].push_back(v);
                    }
                }
                iterator_ = fields_.end();
            }

            HttpHeadIArchive(
                fields_map const & fields, 
                bool clear_optional = false)
                : clear_optional_(clear_optional)
                , fields_(fields)
                , iterator_(fields_.end())
                , sub_level_(0)
                , sub_item_(0)
            {
            }

        public:
            /// 从流中读出变量
            template<class T>
            void load(
                T & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ == fields_.end() 
                        || iterator_->second.size()) {
                        state(2);
                    } else if (iterator_->second.size() == 1) {
                        error_code ec = parse2(iterator_->second.front(), t);
                        if (ec)
                            state(2);
                    } else {
                        state(2);
                    }
                } else if (sub_level_ == 2) {
                    if (iterator_ != fields_.end()) {
                        if (sub_item_ == size_t(-1)) {
                            t = parse<T>(format(iterator_->second.size()));
                            sub_item_ = 0;
                        } else if (sub_item_ < iterator_->second.size()) {
                            error_code ec = parse2(iterator_->second[sub_item_], t);
                            if (ec)
                                state(2);
                            ++sub_item_;
                        } else {
                            state(2);
                        }
                    }
                } else {
                    state(2);
                }
            }

            void load(
                std::string & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ == fields_.end() 
                        || iterator_->second.empty()) {
                        state(2);
                    } else if (iterator_->second.size() == 1) {
                        t = iterator_->second.front();
                    } else {
                        state(2);
                    }
                } else if (sub_level_ == 2) {
                    if (iterator_ != fields_.end()) {
                        if (sub_item_ < iterator_->second.size()) {
                            t = iterator_->second[sub_item_];
                            ++sub_item_;
                        } else {
                            state(2);
                        }
                    }
                } else {
                    state(2);
                }
            }

            template<class T>
            void load(
                boost::optional<T> & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ == fields_.end() 
                        || iterator_->second.empty()) {
                        t.reset();
                    } else if (iterator_->second.size() == 1) {
                        T t1;
                        error_code ec = parse2(iterator_->second.front(), t1);
                        if (ec) {
                            state(2);
                        } else {
                            t.reset(t1);
                        }
                    } else {
                        state(2);
                    }
                } else {
                    state(2);
                }
            }

            using BasicIArchive<HttpHeadIArchive>::load;

            void load_other(
                fields_map & other)
            {
                if (state())
                    return;
                for (fields_map_iterator i = fields_.begin(); i != fields_.end(); ++i) {
                    other[i->first] = i->second;
                }
            }

            void load_start(
                std::string const & name)
            {
                if (sub_level_ == 1)
                    iterator_ = fields_.find(name);
            }

            void load_end(
                std::string const & name)
            {
                if (sub_level_ == 1) {
                    if (iterator_ != fields_.end()) {
                        fields_.erase(iterator_);
                        iterator_ = fields_.end();
                    }
                }
            }

            void sub_start()
            {
                if (++sub_level_ == 2) {
                    sub_item_ = size_t(-1);
                }
            }

            void sub_end()
            {
                --sub_level_;
            }

        private:
            bool clear_optional_;
            fields_map fields_;
            fields_map_iterator iterator_;
            size_t sub_level_;
            size_t sub_item_;
        };

        class HttpHeadOArchive
            : public BasicOArchive<HttpHeadOArchive>
        {
            friend class BasicOArchive<HttpHeadOArchive>;
            friend struct LoadAccess;

            typedef std::map<std::string, std::vector<std::string> > fields_map;
            typedef fields_map::iterator fields_map_iterator;
            typedef fields_map::const_iterator fields_map_const_iterator;

        public:
            HttpHeadOArchive(
                std::ostream & os)
                : iterator_(fields_.end())
                , os_(os)
                , sub_level_(0)
                , sub_item_(0)
            {
            }

            ~HttpHeadOArchive()
            {
                if (state())
                    return;
                for (fields_map_iterator i = fields_.begin(); i != fields_.end(); ++i) {
                    for (std::vector<std::string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
                        os_ << i->first << ": " << *j << "\r\n";
                }
            }

        public:
            /// 从流中读出变量
            template<class T>
            void save(
                T const & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ != fields_.end()) {
                        if (iterator_->second.empty()) {
                            std::string str;
                            error_code ec = format2(str, t);
                            if (ec) {
                                state(2);
                            } else {
                                iterator_->second.push_back(str);
                            }
                        } else {
                            state(2);
                        }
                    }
                } else if (sub_level_ == 2) {
                    if (iterator_ != fields_.end()) {
                        if (sub_item_ == size_t(-1)) {
                            iterator_->second.resize(parse<size_t>(format(t)));
                            sub_item_ = 0;
                        } else if (sub_item_ < iterator_->second.size()) {
                            error_code ec = format2(iterator_->second[sub_item_], t);
                            if (ec)
                                state(2);
                            ++sub_item_;
                        } else {
                            state(2);
                        }
                    }
                } else {
                    state(2);
                }
            }

            void save(
                std::string const & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ != fields_.end()) {
                        if (iterator_->second.empty()) {
                            iterator_->second.push_back(t);
                        } else {
                            state(2);
                        }
                    }
                } else if (sub_level_ == 2) {
                    if (iterator_ != fields_.end()) {
                        if (sub_item_ < iterator_->second.size()) {
                            iterator_->second[sub_item_] = t;
                            ++sub_item_;
                        } else {
                            state(2);
                        }
                    }
                } else {
                    state(2);
                }
            }

            template<class T>
            void save(
                boost::optional<T> const & t)
            {
                if (sub_level_ == 0) {
                    state(2);
                } else if (sub_level_ == 1) {
                    if (iterator_ != fields_.end()) {
                        if (iterator_->second.empty()) {
                            if (t.is_initialized()) {
                                iterator_->second.resize(1);
                                error_code ec = format2(iterator_->second.front(), t.get());
                                if (ec) {
                                    state(2);
                                }
                            }
                        } else {
                            state(2);
                        }
                    }
                } else {
                    state(2);
                }
            }

            void save_other(
                fields_map const & other)
            {
                for (fields_map_const_iterator i = other.begin(); state() == 0 && i != other.end(); ++i) {
                    if (!fields_.insert(*i).second) {
                        state(2);
                    }
                }
            }

            using BasicOArchive<HttpHeadOArchive>::save;

            void save_start(
                std::string const & name)
            {
                if (sub_level_ == 1) {
                    iterator_ = fields_.find(name);
                    if (iterator_ == fields_.end())
                        iterator_ = fields_.insert(std::make_pair(name, std::vector<std::string>())).first;
                }
            }

            void save_end(
                std::string const & name)
            {
                if (sub_level_ == 1) {
                    iterator_ = fields_.end();
                }
            }

            void sub_start()
            {
                if (++sub_level_ == 2) {
                    sub_item_ = size_t(-1);
                }
            }

            void sub_end()
            {
                --sub_level_;
            }

        private:
            fields_map fields_;
            fields_map_iterator iterator_;
            std::ostream & os_;
            size_t sub_level_;
            size_t sub_item_;
        };

        class HttpHeadVisitor
            : public BasicIArchive<HttpHeadVisitor>
        {
            friend class BasicIArchive<HttpHeadVisitor>;
            friend struct util::archive::LoadAccess;

        public:
            typedef std::map<std::string, std::vector<std::string> > fields_map;
            typedef fields_map::iterator fields_map_iterator;
            typedef fields_map::const_iterator fields_map_const_iterator;

        public:
            HttpHeadVisitor(
                HttpStringField & field)
                : field_(field)
            {
            }

            void visit(HttpHead & head)
            {
                this->operator >> (head);
            }

            void visit_other(
                fields_map & other)
            {
                if (!handler_.get())
                    handler_.reset(new OtherHandler(field_.name(), other));
            }

            std::auto_ptr<HttpStringField::Handler> & handler()
            {
                return handler_;
            }

        private:
            /// 从流中读出变量
            template <typename T>
            class RegularHandler
                : public HttpStringField::Handler
            {
            public:
                RegularHandler(
                    T & t)
                    : t_(t)
                {
                }

            private:
                virtual void set(
                    bool empty, 
                    std::string const & value)
                {
                    if (!empty)
                        parse2(value, t_);
                }

                virtual void get(
                    bool & empty, 
                    std::string & value) const
                {
                    empty = false;
                    format2(value, t_);
                }

            private:
                T & t_;
            };

            template <typename T>
            class OptionalHandler
                : public HttpStringField::Handler
            {
            public:
                OptionalHandler(
                    boost::optional<T> & t)
                    : t_(t)
                {
                }

            private:
                virtual void set(
                    bool empty, 
                    std::string const & value)
                {
                    std::string str;
                    if (empty)
                        t_.reset();
                    else
                        t_.reset(parse<T>(value));
                }

                virtual void get(
                    bool & empty, 
                    std::string & value) const
                {
                    if (t_.is_initialized()) {
                        empty = false;
                        format2(value, t_.get());
                    } else {
                        empty = true;
                        value.clear();
                    }
                }

            private:
                boost::optional<T> & t_;
            };

            class OtherHandler
                : public HttpStringField::Handler
            {
            public:
                OtherHandler(
                    std::string const & name, 
                    fields_map & t)
                    : name_(name)
                    , t_(t)
                {
                }

            private:
                virtual void set(
                    bool empty, 
                    std::string const & value)
                {
                    std::string str;
                    if (empty) {
                        t_.erase(name_);
                    } else {
                        parse2(value, t_[name_]);
                    }
                }

                virtual void get(
                    bool & empty, 
                    std::string & value) const
                {
                    fields_map_const_iterator i = t_.find(name_);
                    if (i == t_.end()) {
                        empty = true;
                        value.clear();
                    } else {
                        empty = false;
                        format2(value, i->second);
                    }
                }

            private:
                std::string name_;
                fields_map & t_;
            };

        private:
            template<class T>
            void load_wrapper(
                util::serialization::NVPair<T> & nvp)
            {
                if (nvp.name() == field_.name()) {
                    handler_.reset(get_handler(nvp.data()));
                }
            }

            template<class T>
            HttpStringField::Handler * get_handler(
                T & t)
            {
                return new RegularHandler<T>(t);
            }

            template<class T>
            HttpStringField::Handler * get_handler(
                boost::optional<T> & t)
            {
                return new OptionalHandler<T>(t);
            }

        private:
            HttpStringField & field_;
            std::auto_ptr<HttpStringField::Handler> handler_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_HTTP_HTTP_ARCHIVE_H_
