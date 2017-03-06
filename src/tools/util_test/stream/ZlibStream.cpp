// compress.cpp
#include "tools/util_test/Common.h"
#include "util/stream/StreamTransfer.h"
using namespace util::stream;

using namespace framework::configure;

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string>
#include "util/stream/ZlibStream.h"

static const int BUF_SIZE = 10240;

void test_compress_zlib()
{
    //char pData[BUF_SIZE*5] = { 0 }; 
    //unsigned char  odata[BUF_SIZE*5] = { 0 }; 
    //size_t nodata = BUF_SIZE*5; 
    //FILE* fp= NULL; 
    //size_t len;

    //memset(pData,0,sizeof(pData));
    //fp = fopen("a.gz","rb"); 
    //if(fp)
    //{ 
    //    len = fread(pData,1,sizeof(pData)-1,fp); 
    //    fclose(fp); 
    //    gzunzip((unsigned char *)pData, len, (unsigned char *)odata, &nodata);
    //    fprintf(stdout, "%s\n", odata); 
    //} 
}

extern unsigned char t1[];
extern unsigned int  t1_len;
extern unsigned char t1_gz[];
extern unsigned int  t1_gz_len;
extern unsigned int  t1_gz_header_len;

extern unsigned char t2[];
extern unsigned int  t2_len;
extern unsigned char t2_gz[];
extern unsigned int  t2_gz_len;
extern unsigned int  t2_gz_header_len;

typedef struct {
    unsigned char* uncompressed;
    unsigned int   uncompressed_len;
    unsigned char* compressed;
    unsigned int   compressed_len;
    unsigned int   compressed_header_len;
} TestVector;

TestVector TestVectors[] = {
    {t1, t1_len, t1_gz, t1_gz_len, t1_gz_header_len},
    {t2, t2_len, t2_gz, t2_gz_len, t2_gz_header_len},
};

#define CHECK(x) {                                  \
    if (!(x)) {                                     \
    printf("TEST FAILED line %d\n", __LINE__);      \
    return -1;                                         \
    }                                               \
}

using namespace util::stream;
using namespace util::stream::helper;
int test_gunzip()
{
    for (unsigned int t = 0; t < sizeof(TestVectors) / sizeof(TestVectors[0]); t++) {
        TestVector * v = &TestVectors[t];
        MemoryStream * ms_gz = new MemoryStream(v->compressed, v->compressed_len);
        InputStreamReference ms_gz_ref(ms_gz);
        ZipInflatingInputStream ziis(ms_gz_ref);
        DataBuffer buffer;
        boost::uint64_t position = 0;
        bool expect_eos = false;
        boost::system::error_code result;

        for (;;) {
            boost::uint32_t chunk = 400000;
            buffer.set_data_size(chunk);
            boost::uint32_t bytes_read = 0;
            result = ziis.read(buffer.use_data(), chunk, &bytes_read);
            if (expect_eos) {
                CHECK(result == error::compress_eos);
                break;
            }
            if (result == error::compress_eos) {
                CHECK(position == v->uncompressed_len);
            } else {
                CHECK(result == error::compress_success);
                buffer.use_data()[bytes_read] = '\0';
                fprintf(stdout, "%s\n", buffer.get_data());
            }
            CHECK(bytes_read <= chunk);
            if (bytes_read != chunk) expect_eos = true;
            CHECK(memcmp(v->uncompressed+position, 
                buffer.get_data(),
                bytes_read) == 0);
            position += bytes_read;
        }
        CHECK(position == v->uncompressed_len);
    }
    return 0;
}

int test_gzfile()
{
    char pData[BUF_SIZE*5] = { 0 }; 
    unsigned char  odata[BUF_SIZE*5] = { 0 }; 
    size_t nodata = BUF_SIZE*5; 
    FILE* fp= NULL; 
    size_t len;

    memset(pData,0,sizeof(pData));
    fp = fopen("a.gz","rb"); 
    if(fp)
    { 
        len = fread(pData,1,sizeof(pData)-1,fp); 
        fclose(fp); 
        MemoryStream * ms = new MemoryStream(pData, len - 128);
        InputStreamReference ms_gz_ref(ms);
        ZipInflatingInputStream ziis(ms_gz_ref);
        DataBuffer buffer;
        boost::uint64_t position = 0;
        bool expect_eos = false;
        boost::system::error_code result;
        std::string str;

        for (;;) {
            boost::uint32_t chunk = 128;
            buffer.set_data_size(chunk);
            boost::uint32_t bytes_read = 0;
            result = ziis.read(buffer.use_data(), chunk, &bytes_read);
            if (expect_eos) {
                CHECK(result == error::compress_eos);
                break;
            }
            if (result == error::compress_eos) {
            } else {
                CHECK(result == error::compress_success);
                buffer.use_data()[bytes_read] = '\0';
                str += (char *)buffer.get_data();
            }
            CHECK(bytes_read <= chunk);
            if (bytes_read != chunk) expect_eos = true;
            position += bytes_read;
        }
        str[position] = '\0';

        fprintf(stdout, "%s\n", str.c_str()); 

        InputStreamReference ms_gz_ref1(new MemoryStream(pData + len - 128, 128));

        ziis.set_read_source(ms_gz_ref1);
        expect_eos = false;

        position = 0;
        str = "";
        for (;;) {
            boost::uint32_t chunk = 128;
            buffer.set_data_size(chunk);
            boost::uint32_t bytes_read = 0;
            result = ziis.read(buffer.use_data(), chunk, &bytes_read);
            if (expect_eos) {
                CHECK(result == error::compress_eos);
                break;
            }
            if (result == error::compress_eos) {
            } else {
                CHECK(result == error::compress_success);
                buffer.use_data()[bytes_read] = '\0';
                str += (char *)buffer.get_data();
            }
            CHECK(bytes_read <= chunk);
            if (bytes_read != chunk) expect_eos = true;
            position += bytes_read;
        }
        str[position] = '\0';
        fprintf(stdout, "%s\n", str.c_str()); 
    } 
}

void test_zlib_stream(Config & conf)
{
    //test_compress_zlib();
    test_gzfile();
    //test_gunzip();
}

static TestRegister test("zlib_stream", test_zlib_stream);
