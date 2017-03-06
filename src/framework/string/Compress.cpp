// Compress.cpp

#include "framework/Framework.h"
#include "framework/string/Compress.h"

namespace framework
{
    namespace string
    {

        typedef unsigned char byte;

        static unsigned _do_compress (byte *in, unsigned in_len, byte *out, unsigned *out_len);

        static unsigned _do_compress (byte *in, unsigned in_len, byte *out, unsigned *out_len)
        {
            static long wrkmem [16384L];
            register byte *ip;
            byte *op;
            byte *in_end = in + in_len;
            byte *ip_end = in + in_len - 13;
            byte *ii;
            byte **dict = (byte **)wrkmem;
            op = out;
            ip = in;
            ii = ip;
            ip += 4;
            for(;;)
            {
                register byte *m_pos;
                unsigned m_off;
                unsigned m_len;
                unsigned dindex;
                dindex = ((0x21*(((((((unsigned)(ip[3])<<6)^ip[2])<<5)^ip[1])<<5)^ip[0]))>>5) & 0x3fff;
                m_pos = dict [dindex];
                if(((unsigned long)m_pos < (unsigned long)in) ||
                    (m_off = (unsigned)((unsigned long)ip-(unsigned long)m_pos) ) <= 0 ||
                    m_off > 0xbfff)
                    goto literal;
                if(m_off <= 0x0800 || m_pos[3] == ip[3])
                    goto try_match;
                dindex = (dindex & 0x7ff ) ^ 0x201f;
                m_pos = dict[dindex];
                if((unsigned long)(m_pos) < (unsigned long)(in) ||
                    (m_off = (unsigned)( (int)((unsigned long)ip-(unsigned long)m_pos))) <= 0 ||
                    m_off > 0xbfff)
                    goto literal;
                if (m_off <= 0x0800 || m_pos[3] == ip[3])
                    goto try_match;
                goto literal;
try_match:
                if(m_pos[0] == ip[0] && m_pos[1] == ip[1] && m_pos[2]==ip[2])
                    goto match;
                literal:
                dict[dindex] = ip;
                ++ip;
                if (ip >= ip_end)
                    break;
                continue;
match:
                dict[dindex] = ip;
                if(ip - ii > 0)
                {
                    register unsigned t = ip - ii;

                    if (t <= 3)
                        op[-2] |= (byte)t;
                    else if(t <= 18)
                        *op++ = (byte)(t - 3);
                    else
                    {
                        register unsigned tt = t - 18;
                        *op++ = 0;
                        while(tt > 255)
                        {
                            tt -= 255;
                            *op++ = 0;
                        }
                        *op++ = (byte)tt;
                    }
                    do *op++ = *ii++; while (--t > 0);
                }
                ip += 3;
                if(m_pos[3] != *ip++ || m_pos[4] != *ip++ || m_pos[5] != *ip++ ||
                    m_pos[6] != *ip++ || m_pos[7] != *ip++ || m_pos[8] != *ip++ )
                {
                    --ip;
                    m_len = ip - ii;

                    if(m_off <= 0x0800 )
                    {
                        --m_off;
                        *op++ = (byte)(((m_len - 1) << 5) | ((m_off & 7) << 2));
                        *op++ = (byte)(m_off >> 3);
                    }
                    else
                        if (m_off <= 0x4000 )
                        {
                            -- m_off;
                            *op++ = (byte)(32 | (m_len - 2));
                            goto m3_m4_offset;
                        }
                        else
                        {
                            m_off -= 0x4000;
                            *op++ = (byte)(16 | ((m_off & 0x4000) >> 11) | (m_len - 2));
                            goto m3_m4_offset;
                        }
                }
                else
                {
                    {
                        byte *end = in_end;
                        byte *m = m_pos + 9;
                        while (ip < end && *m == *ip)
                            m++, ip++;
                        m_len = (ip - ii);
                    }

                    if(m_off <= 0x4000)
                    {
                        --m_off;
                        if (m_len <= 33)
                            *op++ = (byte)(32 | (m_len - 2));
                        else
                        {
                            m_len -= 33;
                            *op++=32;
                            goto m3_m4_len;
                        }
                    }
                    else
                    {
                        m_off -= 0x4000;
                        if(m_len <= 9)
                            *op++ = (byte)(16|((m_off & 0x4000) >> 11) | (m_len - 2));
                        else
                        {
                            m_len -= 9;
                            *op++ = (byte)(16 | ((m_off & 0x4000) >> 11));
m3_m4_len:
                            while (m_len > 255)
                            {
                                m_len -= 255;
                                *op++ = 0;
                            }
                            *op++ = (byte)m_len;
                        }
                    }
m3_m4_offset:
                    *op++ = (byte)((m_off & 63) << 2);
                    *op++ = (byte)(m_off >> 6);
                }
                ii = ip;
                if (ip >= ip_end)
                    break;
            }
            *out_len = op - out;
            return (unsigned) (in_end - ii);
        }

        int compress(void *in, unsigned in_len, void *out)
        {
            byte *op = (byte*)out;
            unsigned t,out_len;
            if (in_len <= 13)
                t = in_len;
            else 
            {
                t = _do_compress ((byte*)in,in_len,op,&out_len);
                op += out_len;
            }
            if (t > 0)
            {
                byte *ii = (byte*)in + in_len - t;
                if (op == (byte*)out && t <= 238)
                    *op++ = (byte) ( 17 + t );
                else
                    if (t <= 3)
                        op[-2] |= (byte)t ;
                    else
                        if (t <= 18)
                            *op++ = (byte)(t-3);
                        else
                        {
                            unsigned tt = t - 18;
                            *op++ = 0;
                            while (tt > 255) 
                            {
                                tt -= 255;
                                *op++ = 0;
                            }
                            *op++ = (byte)tt;
                        }
                        do *op++ = *ii++; while (--t > 0);
            }
            *op++ = 17;
            *op++ = 0;
            *op++ = 0;
            return (op - (byte*)out);
        }

        int decompress (void *in, unsigned in_len, void *out)
        {
            register byte *op;
            register byte *ip;
            register unsigned t;
            register byte *m_pos;
            byte *ip_end = (byte*)in + in_len;
            op = (byte*)out;
            ip = (byte*)in;
            if(*ip > 17)
            {
                t = *ip++ - 17;
                if (t < 4)
                    goto match_next;
                do *op++ = *ip++; while (--t > 0);
                goto first_literal_run;
            }
            for(;;)
            {
                t = *ip++;
                if (t >= 16) goto match;
                if (t == 0)
                {
                    while (*ip == 0)
                    {
                        t += 255;
                        ip++;
                    }
                    t += 15 + *ip++;
                }
                * (unsigned *) op = * ( unsigned *) ip;
                op += 4; ip += 4;
                if (--t > 0)
                {
                    if (t >= 4)
                    {
                        do
                        {
                            * (unsigned * ) op = * ( unsigned * ) ip;
                            op += 4; ip += 4; t -= 4;
                        } while (t >= 4);
                        if (t > 0) do *op++ = *ip++; while (--t > 0);
                    }
                    else
                        do *op++ = *ip++; while (--t > 0);
                }
first_literal_run:
                t = *ip++;
                if (t >= 16)
                    goto match;
                m_pos = op - 0x0801;
                m_pos -= t >> 2;
                m_pos -= *ip++ << 2;
                *op++ = *m_pos++; *op++ = *m_pos++; *op++ = *m_pos;
                goto match_done;
                for(;;)
                {
match:
                    if (t >= 64)
                    {
                        m_pos = op - 1;
                        m_pos -= (t >> 2) & 7;
                        m_pos -= *ip++ << 3;
                        t = (t >> 5) - 1;
                        goto copy_match;
                    }
                    else 
                        if (t >= 32)
                        {
                            t &= 31;
                            if (t == 0)    
                            {
                                while (*ip == 0) 
                                {
                                    t += 255;
                                    ip++;
                                }
                                t += 31 + *ip++;
                            }
                            m_pos = op - 1;
                            m_pos -= (* ( unsigned short * ) ip) >> 2;
                            ip += 2;
                        }
                        else
                            if (t >= 16) 
                            {
                                m_pos = op;
                                m_pos -= (t & 8) << 11;
                                t &= 7;
                                if (t == 0)
                                {
                                    while (*ip == 0)
                                    {
                                        t += 255;
                                        ip++;
                                    }
                                    t += 7 + *ip++;
                                }
                                m_pos -= (* ( unsigned short *) ip) >> 2;
                                ip += 2;
                                if (m_pos == op)
                                    goto eof_found;
                                m_pos -= 0x4000;
                            }
                            else 
                            {
                                m_pos = op - 1;
                                m_pos -= t >> 2;
                                m_pos -= *ip++ << 2;
                                *op++ = *m_pos++; *op++ = *m_pos;
                                goto match_done;
                            }
                            if (t >= 6 && (op - m_pos) >= 4) 
                            {
                                * (unsigned *) op = * ( unsigned *) m_pos;
                                op += 4; m_pos += 4; t -= 2;
                                do
                                {
                                    * (unsigned *) op = * ( unsigned *) m_pos;
                                    op += 4; m_pos += 4; t -= 4;
                                }while (t >= 4);
                                if (t > 0)
                                    do *op++ = *m_pos++; while (--t > 0);
                            }
                            else
                            {
copy_match:
                                *op++ = *m_pos++; *op++ = *m_pos++;
                                do *op++ = *m_pos++; while (--t > 0);
                            }
match_done:
                            t = ip[-2] & 3;
                            if (t == 0)    break;
match_next:
                            do *op++ = *ip++; while (--t > 0);
                            t = *ip++;
                }
            }
eof_found:
            if (ip != ip_end) return -1;
            return (op - (byte*)out);
        }

    } // namespace string
} // namespace framework
