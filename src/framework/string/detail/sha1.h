#ifndef SHA1_H
#define SHA1_H

/*
SHA-1 in C
By Steve Reid <sreid@sea-to-sky.net>
100% Public Domain

-----------------
23 Apr 2001 version from http://sea-to-sky.net/~sreid/
Modified slightly to take advantage of autoconf.
See sha1.c for full history comments.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned long state[5];
    unsigned long count[2];
    unsigned char buffer[64];
} framework_string_SHA1_CTX;

void framework_string_SHA1Transform(unsigned long state[5], unsigned char buffer[64]);
void framework_string_SHA1Init(framework_string_SHA1_CTX * context);
void framework_string_SHA1Update(framework_string_SHA1_CTX * context, unsigned char * data, unsigned long len); /* JHB */
void framework_string_SHA1Final(unsigned char digest[20], framework_string_SHA1_CTX * context);

#ifdef __cplusplus
}
#endif

#endif

