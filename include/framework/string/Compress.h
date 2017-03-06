// Compress.h

#ifndef _FRAMEWORK_STRING_COMPRESS_H_
#define _FRAMEWORK_STRING_COMPRESS_H_

namespace framework
{
    namespace string
    {

        int compress(void *in, unsigned in_len, void *out);
        int decompress (void *in, unsigned in_len, void *out);

    } // namespace string
} // namespace framework

#endif // _FRAMEWORK_STRING_COMPRESS_H_
