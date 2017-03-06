// security.cpp

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string>
#include <security/Des.h>
#include <vector>
#include <iostream>
#include "boost/cstdint.hpp"
#include "framework/string/Base64.h"
using namespace security;
using namespace framework::string;

std::vector<std::string> keys;

void test_security_3des()
{
    char* enword = "12345678";//加密字符串
    char key[] = {"\x15\xB9\xFD\xAE\xDA\x40\xF8\x6B\xF7\x1C\x73\x29\x25\x16\x92\x4A\x29\x4F\xC8\xBA\x31\xB6\xE9\xEA"};//加密key
    char iv[]={"\x70\x70\x6C\x69\x76\x65\x6F\x6B"};//向量
    char k[] = {"iJ/Pch9aQh0gusryqDTKzQ=="};
    //char *p = Des::base64decode(k);

    keys.push_back("\x70\x70\x6C\x69\x76\x65\x6F\x6B");
    keys.push_back("\x15\xB9\xFD\xAE\xDA\x40\xF8\x6B\xF7\x1C\x73\x29\x25\x16\x92\x4A\x29\x4F\xC8\xBA\x31\xB6\xE9\xEA");
    keys.push_back("\x29\x02\x8A\x76\x98\xEF\x4C\x6D\x3D\x25\x2F\x02\xF4\xF7\x9D\x58\x15\x38\x9D\xF1\x85\x25\xD3\x26");
    keys.push_back("\xD0\x46\xE6\xB6\xA4\xA8\x5E\xB6\xC4\x4C\x73\x37\x2A\x0D\x5D\xF1\xAE\x76\x40\x51\x73\xB3\xD5\xEC");
    keys.push_back("\x43\x52\x29\xC8\xF7\x98\x31\x13\x19\x23\xF1\x8C\x5D\xE3\x2F\x25\x3E\x2A\xF2\xAD\x34\x8C\x46\x15");
    keys.push_back("\x9B\x29\x15\xA7\x2F\x83\x29\xA2\xFE\x6B\x68\x1C\x8A\xAE\x1F\x97\xAB\xA8\xD9\xD5\x85\x76\xAB\x20");
    keys.push_back("\xB3\xB0\xCD\x83\x0D\x92\xCB\x37\x20\xA1\x3E\xF4\xD9\x3B\x1A\x13\x3D\xA4\x49\x76\x67\xF7\x51\x91");
    keys.push_back("\xAD\x32\x7A\xFB\x5E\x19\xD0\x23\x15\x0E\x38\x2F\x6D\x3B\x3E\xB5\xB6\x31\x91\x20\x64\x9D\x31\xF8");
    keys.push_back("\xC4\x2F\x31\xB0\x08\xBF\x25\x70\x67\xAB\xF1\x15\xE0\x34\x6E\x29\x23\x13\xC7\x46\xB3\x58\x1F\xB0");
    keys.push_back("\x52\x9B\x75\xBA\xE0\xCE\x20\x38\x46\x67\x04\xA8\x6D\x98\x5E\x1C\x25\x57\x23\x0D\xDF\x31\x1A\xBC");
    keys.push_back("\x8A\x52\x9D\x5D\xCE\x91\xFE\xE3\x9E\x9E\xE9\x54\x5D\xF4\x2C\x3D\x9D\xEC\x2F\x76\x7C\x89\xCE\xAB");

    //for( int i = 0; i < strlen(p); ++i)
    //{
    //    printf("%2x ", (unsigned char)p[i]);
    //}
    //printf("\r\n");
    ////Des::padding(PAD_ISO_1,enword,strlen(enword),enword,0x0);
    //char* buffer = new char[200];
    //memset(buffer,0x0,200);
    //char* bufferout = new char[200];
    //memset(bufferout,0x0,200);
    //int PadMode = Des::PAD_ISO_1;
    //{
    //    PadMode = Des::PAD_ISO_1;
    //    Des::triple_des(Des::ENCRYPT,Des::ECB,enword,strlen(enword),key,strlen(key),buffer,200,iv,PadMode);

    //    printf("PAD_ISO_1 ECB: ");
    //    for( int i = 0; i < strlen(buffer); ++i)
    //    {
    //        printf("%2x ", (unsigned char)buffer[i]);
    //    }

    //    printf("Base64 Encoding: %s\t", Des::base64encode(buffer, strlen(buffer)));

    //    Des::triple_des(Des::DECRYPT,Des::ECB,buffer,strlen(buffer),key,strlen(key),bufferout,200,iv,PadMode);
    //    printf("Decrypto:%s\r\n",bufferout);
    //}
    //{
    //    memset(bufferout,0x0,200);
    //    PadMode = Des::PAD_ISO_1;
    //    Des::triple_des(Des::ENCRYPT,Des::CBC,enword,strlen(enword),key,strlen(key),buffer,200,iv,PadMode);

    //    printf("PAD_ISO_1 CBC: ");
    //    for( int i = 0; i < strlen(buffer); ++i)
    //    {
    //        printf("%2x ", (unsigned char)buffer[i]);
    //    }

    //    printf("Base64 Encoding: %s\t", Des::base64encode(buffer, strlen(buffer)));

    //    Des::triple_des(Des::DECRYPT,Des::CBC,buffer,strlen(buffer),key,strlen(key),bufferout,200,iv,PadMode);
    //    printf("Decrypto:%s\r\n",bufferout);
    //}
    //{
    //    memset(bufferout,0x0,200);
    //    PadMode = Des::PAD_ISO_2;
    //    Des::triple_des(Des::ENCRYPT,Des::ECB,enword,strlen(enword),key,strlen(key),buffer,200,iv,PadMode);

    //    printf("PAD_ISO_2 ECB: ");
    //    for( int i = 0; i < 8; ++i)
    //    {
    //        printf("%2x ", (unsigned char)buffer[i]);
    //    }

    //    printf("Base64 Encoding: %s\t", Des::base64encode(buffer, strlen(buffer)));

    //    Des::triple_des(Des::DECRYPT,Des::ECB,buffer,strlen(buffer),key,strlen(key),bufferout,200,iv,PadMode);
    //    printf("Decrypto:%s\r\n",bufferout);
    //}
    //{
    //    memset(bufferout,0x0,200);
    //    PadMode = Des::PAD_ISO_2;
    //    Des::triple_des(Des::ENCRYPT,Des::CBC,enword,strlen(enword),key,strlen(key),buffer,200,iv,PadMode);

    //    printf("PAD_ISO_2 CBC: ");
    //    for( int i = 0; i < strlen(buffer); ++i)
    //    {
    //        printf("%2x ", (unsigned char)buffer[i]);
    //    }

    //    printf("Base64 Encoding: %s\t", Des::base64encode(buffer, strlen(buffer)));

    //    Des::triple_des(Des::DECRYPT,Des::CBC,buffer,strlen(buffer),key,strlen(key),bufferout,200,iv,PadMode);
    //    printf("Decrypto:%s\r\n",bufferout);
    //}
    //{
    //    memset(bufferout,0x0,200);
    //    PadMode = Des::PAD_PKCS_7;
    //    Des::triple_des(Des::ENCRYPT,Des::ECB,enword,strlen(enword),key,strlen(key),buffer,200,iv,PadMode);

    //    printf("PAD_PKCS_7 ECB: ");
    //    for( int i = 0; i < strlen(buffer); ++i)
    //    {
    //        printf("%2x ", (unsigned char)buffer[i]);
    //    }

    //    printf("Base64 Encoding: %s\t", Des::base64encode(buffer, strlen(buffer)));

    //    Des::triple_des(Des::DECRYPT,Des::ECB,buffer,strlen(buffer),key,strlen(key),bufferout,200,iv,PadMode);
    //    printf("Decrypto:%s\r\n",bufferout);
    //}
    //{
    //    memset(bufferout,0x0,200);
    //    PadMode = Des::PAD_PKCS_7;
    //    Des::triple_des(Des::ENCRYPT,Des::CBC,enword,strlen(enword),key,strlen(key),buffer,200,iv,PadMode);

    //    printf("PAD_PKCS_7 CBC: ");
    //    for( int i = 0; i < strlen(buffer); ++i)
    //    {
    //        printf("%2x ", (unsigned char)buffer[i]);
    //    }

    //    printf("Base64 Encoding: %s\t", Des::base64encode(buffer, strlen(buffer)));

    //    Des::triple_des(Des::DECRYPT,Des::CBC,buffer,strlen(buffer),key,strlen(key),bufferout,200,iv,PadMode);
    //    printf("Decrypto:%s\r\n",bufferout);
    //}
    //{
    //    memset(bufferout,0x0,200);
    //    Des::pptv_3_des(Des::ENCRYPT, enword, strlen(enword), key, strlen(key), buffer, 200);
    //    printf("PPTV Encode: ");

    //    for( int i = 0; i < strlen(buffer); ++i)
    //    {
    //        printf("%2x ", (unsigned char)buffer[i]);
    //    }

    //    printf("Base64 Encoding: %s\t", Des::base64encode(buffer, strlen(buffer)));

    //    Des::pptv_3_des(Des::DECRYPT, buffer, strlen(buffer), key, strlen(key), bufferout, 200);
    //    printf("Decrypto:%s\r\n",bufferout);
    //}

    //{
    //    memset(bufferout,0x0,200);
    //    std::string instr = "1234567890";
    //    std::string outstr, outstr1;
    //    std::string mykey = "\x15\xB9\xFD\xAE\xDA\x40\xF8\x6B\xF7\x1C\x73\x29\x25\x16\x92\x4A\x29\x4F\xC8\xBA\x31\xB6\xE9\xEA";
    //    Des::pptv_3_des(Des::ENCRYPT, instr, mykey, outstr);
    //    printf("PPTV Encode: ");
    //    printf("Base64 Encoding: %s\t", Des::base64encode(outstr.c_str(), outstr.size()));

    //    Des::pptv_3_des(Des::DECRYPT, outstr, mykey, outstr1);
    //    printf("Decrypto:%s\r\n",outstr1.c_str());
    //    outstr = "UPv/9UaOjLPMhtvTeCWleMuCQUGWM23kATwqN4WU8BM=";
    //    Des::pptv_3_des(Des::DECRYPT, Des::base64decode((char *)outstr.c_str()), keys[2], outstr1);
    //    printf("Decrypto:%s\r\n",outstr1.c_str());
    //    outstr = "X207eqldlNX9Nu7BdjwvpibZFDKcl+w+3GWBEBQGC5k=";
    //    Des::pptv_3_des(Des::DECRYPT, Des::base64decode((char *)outstr.c_str()), keys[5], outstr1);
    //    printf("Decrypto:%s\r\n",outstr1.c_str());
    //}
}

int main( int argc, char ** argv )
{
    test_security_3des();

    if (argc != 4) return 0;

    int ki = argv[1][0] - 0x30;
    std::string type = argv[2];
    std::string code = argv[3];
    std::cout << "\n====================================\ntype = " << type << " code = " << code << " key = " << ki << std::endl;

    char buffer[200] = {0};
    memset(buffer, 0, 200);
    if (type == "e")
    {
        Des::triple_des(Des::ENCRYPT,Des::CBC,code.c_str(),(unsigned int)code.size(),keys[ki-1].c_str(),24,buffer,200,keys[0].c_str(),Des::PAD_PKCS_7);
        printf("Encode %s = %s\n", code.c_str(), Des::base64encode((const char *)buffer, (int)strlen(buffer)));
    }
    else if(type == "d")
    {
        //std::string decode = Des::base64decode((char *)code.c_str());
        //Des::triple_des(Des::DECRYPT,Des::CBC,decode.c_str(), decode.size(),keys[ki-1].c_str(),24,buffer,200,iv,Des::PAD_PKCS_7);
        //printf("Decode : %s\n", buffer);

        if (Des::pptv_3_des_d(code.c_str(), code.size(), keys[ki].c_str(), 24, buffer, 200) || Des::pptv_3_des_d(code.c_str(), code.size(), keys[ki].c_str(), 24, buffer, 200))
        {
            printf("Decode : %s\n", buffer);
        }
        else
            printf("Error\n");

    }
    return 0;
}
