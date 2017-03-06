// RtspFieldRange.h

#ifndef _UTIL_PROTOCOL_RTSP_FIELD_RANGE_H_
#define _UTIL_PROTOCOL_RTSP_FIELD_RANGE_H_

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
        namespace rtsp_field
        {

            class Range
            {
            public:
                Range(
                    float b, 
                    float e)
                {
                    add_range(b, e);
                }

                Range(
                    float b = 0)
                {
                    add_range(b);
                }

                void add_range(
                    float b)
                {
                    units_.push_back(Unit(b, b - 1.0));
                }

                void add_range(
                    float b, 
                    float e)
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
                        float b, 
                        float e)
                        : b_(b)
                        , e_(e)
                    {
                    }

                public:
                    float begin() const
                    {
                        return b_;
                    }

                    float end() const
                    {
                        return e_;
                    }

                    bool has_end() const
                    {
                         return e_ >= b_;
                    }

                public:
                    std::string to_string() const
                    {
                        using namespace framework::string;

                        if (b_ >= 0) {
                            if (has_end()) {
                                return format(b_) + "-" + format(e_);
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
                            e_ = b_ - 1.0;
                        } else {
                            ec = parse2(str.substr(0, p), b_);
                            if (!ec)
                                ec = parse2(str.substr(p + 1), e_);
                        }
                        return ec;
                    }

                private:
                    float b_;
                    float e_;
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
            };

        }

    } // namespace protocol
} // namespace util

namespace util
{
    namespace protocol
    {
        namespace rtsp_field
        {

            inline std::string Range::to_string() const
            {
                using namespace framework::string;
                return join(units_.begin(), units_.end(), ",", "npt=");
            }

            inline boost::system::error_code Range::from_string(
                std::string const & str)
            {
                units_.clear();
                using namespace framework::string;
                return slice<Range::Unit>(str, std::inserter(units_, units_.end()), ",", "npt=");
            }

        }
    }
}

#endif // _UTIL_PROTOCOL_RTSP_FIELD_RANGE_H_
