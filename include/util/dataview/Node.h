// Node.h

#ifndef _UTIL_DATAVIEW_NODE_H_
#define _UTIL_DATAVIEW_NODE_H_

#include "util/dataview/Data.h"

namespace util
{
    namespace dataview
    {

        struct Node
        {
            enum TypeEnum
            {
                t_cat, 
                t_data, 
            };

            Node(
                std::string const & name = "", 
                TypeEnum type = t_cat)
                : name(name)
                , type(type)
            {
            }

            virtual ~Node()
            {
            }

            std::string name;
            TypeEnum type;

            template<typename Archive>
            void serialize(Archive & ar);
        };

        struct Path
            : std::vector<std::string>
        {
        public:
            Path() {}

            Path(
                char const * name)
            {
                push_back(name);
            }

            Path(
                std::string const & name)
            {
                push_back(name);
            }

            Path(
                std::vector<std::string> const & vec)
                : std::vector<std::string>(vec)
            {
            }

            Path(
                Path const & p, 
                Path const & r)
                : std::vector<std::string>(p.begin(), p.end())
            {
                insert(end(), r.begin(), r.end());
            }

        public:
            Path sub_path(
                size_t l = 1) const
            {
                assert(l <= size());
                if (l > size())
                    l = size();
                return Path(*this, l);
            }

            std::string leaf() const
            {
                if (!empty()) {
                    return back();
                } else {
                    return std::string();
                }
            }

            std::string to_string(
                std::string const & spliter = "/") const;

            boost::system::error_code from_string(
                std::string const & path_str, 
                std::string const & spliter = "/");

        public:
            Path operator / (
                Path const & r) const
            {

                return Path(*this, r);
            }

            Path operator -- ()
            {
                if (!empty()) {
                    Path temp(*this);
                    pop_back();
                    return temp;
                } else {
                    return *this;
                }
            }

            Path & operator -- (
                int)
            {
                if (!empty()) {
                    pop_back();
                }
                return *this;
            }

        public:
            template<typename Archive>
            void serialize(Archive & ar);

        private:
            Path(
                Path const & p, 
                size_t l)
                : std::vector<std::string>(p.begin() + l, p.end())
            {
            }
        };

    }
}

#endif // _UTIL_DATAVIEW_NODE_H_
