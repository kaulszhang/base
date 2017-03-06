// CheckSum.h

#ifndef _UTIL_PROTOCOL_PPTV_CHECK_SUM_H_
#define _UTIL_PROTOCOL_PPTV_CHECK_SUM_H_

namespace util
{
    namespace protocol
    {

        namespace pptv
        {
            
            inline boost::uint16_t check_sum(
                boost::uint16_t const * buf, 
                size_t len) // in bytes
            {
                assert(((unsigned long)buf & 1) == 0);
                boost::uint16_t n = 0;
                boost::uint32_t sum = 0;
                boost::uint16_t const * p = buf;
                boost::uint16_t const * e = buf + (len >> 1);
                for (; p < e; ++p) {
                    n = n ^ (*p);
                    sum += n;
                }
                if (len & 1) {
                    union {
                        boost::uint8_t n1[2];
                        boost::uint16_t n2;
                    } nn;
                    nn.n1[0] = *(boost::uint8_t *)p;
                    nn.n1[1] = 'I';
                    n = n ^ nn.n2;
                    sum += n;
                }
                sum = (sum >> 16) + (sum & 0xffff);
                return (boost::uint16_t)(~sum);
            }

        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_PPTV_CHECK_SUM_H_
