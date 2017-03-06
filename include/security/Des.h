// Des.h
#ifndef _SECURITY_DES_H_
#define _SECURITY_DES_H_

#include <string>

namespace security
{
    const int MAXBLOCK = 1024;
    typedef char (*pSubKey)[16][48];

    class Des
    {
    public:
        enum _eEDType
        {
            ENCRYPT = 0,
            DECRYPT = 1,
        };

        enum _eCBType
        {
            ECB = 0,
            CBC = 1,
        };

        enum _ePaddingType
        {
            PAD_ISO_1   = 0,
            PAD_ISO_2   = 1,
            PAD_PKCS_7  = 2,
        };
    public:
        Des();
        ~Des();

    public:
        //Ω‚√‹
        static int pptv_3_des_d(const char *In, unsigned int in_len, const char *Key, unsigned int key_len, char* Out, unsigned int out_len);
        //º”√‹
        static int pptv_3_des_e(const char *In, unsigned int in_len, const char *Key, unsigned int key_len, char* Out, unsigned int out_len);
        
        
        static int pptv_3_des(int bType, const char *In, unsigned int in_len, const char *Key, unsigned int key_len, char* Out, unsigned int out_len);
        static bool pptv_3_des(int type, std::string const & in, std::string const & key, std::string & out);

    public:
        static int convert_key(char *iKey, char *oKey);
        static void rsm(char *Text);
        static int padding(int nType,const char* In,unsigned in_len,char* Out,int* padlen);
        static int triple_des(int bType, int bMode, const char *In, unsigned int in_len, const char *Key, unsigned int key_len, char* OutBuf, unsigned int out_len, const char cvecstr[8],int padMode);
        static int one_des(int bType, int bMode, const char *In, unsigned int in_len, const char *Key, unsigned int key_len, char* Out, unsigned int out_len, const char cvecstr[8],int padMode);
        static void des(char Out[8], const char In[8], const pSubKey pSubKey, int Type);
        static char* base64encode(const char *src,int srclen);
        static char* base64decode(char *src, int *out_size);

    private:
        static char *ch64;
        static const char IP_Table[64];
        static const char IPR_Table[64];
        static const char E_Table[48];
        static const char P_Table[32];
        static const char PC1_Table[56];
        static const char PC2_Table[48];
        static const char LOOP_Table[16];
        static const char S_Box[8][4][16];
        static void _F_func(char In[32], const char Ki[48]);
        static void _S_func(char Out[32], const char In[48]);
        static void _transform(char *Out, const char *In, const char *Table, int len);
        static void _xor(char *InA, const char *InB, int len);
        static void _rotateL(char *In, int len, int loop);
        static void _bit2byte(char *Out, const char *In, int bits);
        static void _byte2bit(char *Out, const char *In, int bits);
        static void _set_sub_key(pSubKey pSubKey, const char Key[8]);
    };
}// namespace security

#endif //_SECURITY_DES_H_

