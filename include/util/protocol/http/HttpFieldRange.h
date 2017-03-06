// HttpFieldRange.h

#ifndef _UTIL_PROTOCOL_HTTP_FIELD_RANGE_H_
#define _UTIL_PROTOCOL_HTTP_FIELD_RANGE_H_

#include <framework/string/Format.h>
#include <framework/string/Parse.h>
#include <framework/string/StringToken.h>
#include <framework/string/Slice.h>
#include <framework/string/Join.h>

#include <iterator>

namespace util
{
    namespace protocol
    {
        namespace http_field
        {

            class Range
            {
            public:
                Range(
                    boost::int64_t b, 
                    boost::int64_t e, 
                    bool    r = false)
                    : is_response_(r)
                {
                    add_range(b, e);
                }

                Range(
                    boost::int64_t b = 0,
                    bool    r = false)
                    : is_response_(r)
                {
                    add_range(b);
                }

                void add_range(
                    boost::int64_t b)
                {
                    units_.push_back(Unit(b, b - 1));
                }

                void add_range(
                    boost::int64_t b, 
                    boost::int64_t e)
                {
                    units_.push_back(Unit(b, e));
                }

                std::string to_string() const;

                boost::system::error_code from_string(
                    std::string const & str);

            public:
                struct Unit
                {
                public:
                    Unit()
                        : b_(0)
                        , e_(0)
                    {
                    }

                    Unit(
                        boost::int64_t b, 
                        boost::int64_t e)
                        : b_(b)
                        , e_(e)
                    {
                    }

                public:
                    boost::int64_t begin() const
                    {
                        return b_;
                    }

                    boost::int64_t end() const
                    {
                        return e_;
                    }

                    bool has_end() const
                    {
                         return e_ > b_;
                    }
                    
                public:
                    std::string to_string() const
                    {
                        using namespace framework::string;

                        if (b_ >= 0) {
                            if (has_end()) {
                                return format(b_) + "-" + format(e_ - 1);
                            } else {
                                return format(b_) + "-";
                            }
                        } else {
                            return format(b_);
                        }
                    }

                    boost::system::error_code from_string(
                        std::string const & str)
                    {
                        using namespace framework::string;
                        using namespace framework::system::logic_error;

                        boost::system::error_code ec = succeed;
                        std::string::size_type p = str.find('-');
                        if (p == 0) {
                            ec = parse2(str, b_);
                        } else if (p == str.size() - 1) {
                            ec = parse2(str.substr(0, p), b_);
                        } else {
                            ec = parse2(str.substr(0, p), b_);
                            if (!ec) {
                                ec = parse2(str.substr(p + 1), e_);
                                if (!ec)
                                    ++e_;
                            }
                        }
                        return ec;
                    }

                    boost::int64_t b_;
                    boost::int64_t e_;
                };

            public:
                Unit & operator[](
                    size_t index)
                {
                    return units_[index];
                }

                Unit const & operator[](
                    size_t index) const
                {
                    return units_[index];
                }

            private:
                std::vector<Unit> units_;
                bool           is_response_;
            };

            class ContentRange
                : Range
            {
            public:
                ContentRange(
                    boost::uint64_t beg = 0,
                    boost::uint64_t total = 0)
                    : Range(beg, total, true)
                    , total_(total)
                {
                }

            public:
                boost::uint64_t total() const
                {
                    return total_;
                }

            public:
                std::string to_string() const
                {
                    return Range::to_string() + "/" + framework::string::format(total_);
                }

                boost::system::error_code from_string(
                    std::string const & str)
                {
                    std::string::size_type p = str.find('/');
                    if (p == std::string::npos) {
                        return framework::system::logic_error::invalid_argument;
                    } else {
                        boost::system::error_code ec = Range::from_string(str.substr(0, p));
                        if (!ec)
                            ec = framework::string::parse2(str.substr(p + 1), total_);
                        return ec;
                    }
                }

            private:
                boost::uint64_t total_;
            };

        }

    } // namespace protocol
} // namespace util

namespace util
{
    namespace protocol
    {
        namespace http_field
        {

            inline std::string Range::to_string() const
            {
                using namespace framework::string;
                if (is_response_)
                    return join(units_.begin(), units_.end(), ",", "bytes ");
                else 
                    return join(units_.begin(), units_.end(), ",", "bytes=");
            }

            inline boost::system::error_code Range::from_string(
                std::string const & str)
            {
                units_.clear();
                using namespace framework::string;
                if (is_response_)
                    return slice<Range::Unit>(str, std::inserter(units_, units_.end()), ",", "bytes ");
                else
                    return slice<Range::Unit>(str, std::inserter(units_, units_.end()), ",", "bytes=");
            }

        }
    }
}

#endif // _UTIL_PROTOCOL_HTTP_FIELD_RANGE_H_
