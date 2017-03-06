// RtmpStreamBuffer.h

#ifndef _UTIL_PROTOCOL_RTMP_RTMP_STREAM_BUFFER_H_
#define _UTIL_PROTOCOL_RTMP_RTMP_STREAM_BUFFER_H_

#include "util/protocol/Message.hpp"
#include "util/protocol/rtmp/RtmpMessageParser.h"

namespace util
{
    namespace protocol
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class RtmpStreamBuffer
            : public std::basic_streambuf<_Elem, _Traits>
        {
        public:
            typedef typename std::basic_streambuf<_Elem, _Traits>::char_type char_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::int_type int_type;
            typedef typename std::basic_streambuf<_Elem, _Traits>::traits_type traits_type;

            RtmpStreamBuffer(
                std::basic_streambuf<_Elem, _Traits> & next_layer, 
                RtmpMessageHeader const & header, 
                boost::uint32_t chk_size)
                : next_layer_(next_layer)
                , header_(header)
                , chk_size_(chk_size)
                , chk_left_(chk_size)
                , filling_(0)
            {
                this->setg(NULL, NULL, NULL);
                this->setp(NULL, NULL);
            }

        public:
            std::basic_streambuf<_Elem, _Traits> & next_layer()
            {
                return next_layer_;
            }

            std::streamsize filling() const
            {
                return filling_;
            }

        private:
            virtual std::streamsize xsgetn(
                _Elem * _Ptr,
                std::streamsize _Count)
            {
                size_t left = (size_t)_Count;
                while (left > chk_left_) {
                    std::streamsize n = next_layer_.sgetn(_Ptr, chk_left_);
                    chk_left_ -= n;
                    _Ptr += n;
                    left -= n;
                    if (chk_left_) {
                        return _Count - left;
                    }
                    RtmpChunkBasicHeader header;
                    int_type b = next_layer_.sbumpc();
                    if (_Traits::eq_int_type(_Traits::eof(), b)) {
                        return _Count - left;
                    }
                    header.one_byte = b;
                    ++filling_;
                    if (header.cs_id0 < 2) {
                        if (header.cs_id0 == 0) {
                            header.cs_id1 = next_layer_.sbumpc();
                            ++filling_;
                        } else {
                            boost::uint8_t b1 = next_layer_.sbumpc();
                            boost::uint8_t b2 = next_layer_.sbumpc();
                            ++filling_;
                            ++filling_;
                            header.cs_id2 = (boost::uint16_t)b1 << 8 | b2;;
                        }
                    }
                    assert(header.cs_id() == header_.chunk && header.fmt == 3);
                    chk_left_ = chk_size_;
                }
                std::streamsize n = next_layer_.sgetn(_Ptr, left);
                chk_left_ -= n;
                left -= n;
                return _Count - left;
            }

            virtual std::streamsize xsputn(
                _Elem const * _Ptr,
                std::streamsize _Count)
            {
                size_t left = (size_t)_Count;
                while (left > chk_left_) {
                    std::streamsize n = next_layer_.sputn(_Ptr, chk_left_);
                    chk_left_ -= n;
                    _Ptr += n;
                    left -= n;
                    if (chk_left_) {
                        return _Count - left;
                    }
                    RtmpChunkBasicHeader header(3, header_.chunk);
                    next_layer_.sputc(header.one_byte);
                    ++filling_;
                    if (header.cs_id0 < 2) {
                        if (header.cs_id0 == 0) {
                            next_layer_.sputc(header.cs_id1);
                            ++filling_;
                        } else {
                            boost::uint8_t b1 = header.cs_id2 >> 8;
                            boost::uint8_t b2 = header.cs_id2 & 0xff;
                            next_layer_.sputc(b1);
                            next_layer_.sputc(b2);
                            ++filling_;
                            ++filling_;
                        }
                    }
                    chk_left_ = chk_size_;
                }
                std::streamsize n = next_layer_.sputn(_Ptr, left);
                chk_left_ -= n;
                left -= n;
                return _Count - left;
            }

        private:
            std::basic_streambuf<_Elem, _Traits> & next_layer_;
            RtmpMessageHeader const & header_;
            boost::uint32_t chk_size_;
            boost::uint32_t chk_left_;
            std::streamsize filling_;
        };

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_RTMP_RTMP_STREAM_BUFFER_H_
