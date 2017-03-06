// BasicIArchive.h

#ifndef _UTIL_ARCHIVE_BASIC_ARCHIVE_H_
#define _UTIL_ARCHIVE_BASIC_ARCHIVE_H_

#include "util/serialization/Serialization.h"
#include "util/serialization/NVPair.h"
#include "util/serialization/stl/string.h"
#include "util/serialization/Array.h"

#if (defined _DEBUG) || (defined DEBUG)
#  define SERIALIZATION_DEBUG
#endif

namespace util
{
    namespace archive
    {

        template <typename Archive>
        class BasicArchive
        {
        protected:
            BasicArchive()
                : state_(0)
                , version_(0)
                , context_(NULL)
            {
            }

            /// 获取派生类的指针
            Archive * This()
            {
                return static_cast<Archive *>(this);
            }

        public:
            size_t version() const
            {
                return version_;
            }

            void version(
                size_t v)
            {
                version_ = v;
            }

            void * context() const
            {
                return context_;
            }

            void context(
                void * c)
            {
                context_ = c;
            }

        public:
            operator bool () const
            {
                return state_ == 0;
            }

            bool operator ! () const
            {
                return state_ != 0;
            }

            void fail()
            {
                state_ = 2;
            }

            bool failed()
            {
                return state_ == 2;
            }

            void clear()
            {
                state_ = 0;
            }

#ifdef SERIALIZATION_DEGUG
            std::string failed_item_path() const
            {
                std::string path;
                for (std::vector<std::string>::const_iterator i = path_.begin(); i != path_.end(); ++i) {
                    path += "/";
                    path += *i;
                }
                return path;
            }
#endif
            void sub_start()
            {
            }

            void sub_end()
            {
            }

        protected:
            int state() const
            {
                return state_;
            }

            void state(
                int s)
            {
                state_ = s;
            }

            void path_push()
            {
#ifdef SERIALIZATION_DEGUG
                path_.push_back(t.name());
#endif
            }

            void path_pop()
            {
#ifdef SERIALIZATION_DEGUG
                if (state_ == 0)
                    path_.pop_back();
#endif
            }

        private:
            int state_;
            size_t version_;
            void * context_;
            std::vector<std::string> path_;
        }; // class basic_iarchive

    }  // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_BASIC_ARCHIVE_H_

