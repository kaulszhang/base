// SymbolBuffer.h

#include <stdlib.h>

namespace framework
{
    namespace debuging
    {

        namespace detail
        {

            struct SymbolBuffer
            {
                SymbolBuffer(
                    size_t num)
                    : num_(num)
                    , size_(num * 256)
                    , buffer_(malloc(size_))
                    , addrs_((char **)buffer_)
                    , buf_((char *)buffer_ + sizeof(char *) * num)
                    , left_(size_ - sizeof(char *) * num)
                {
                }

                SymbolBuffer(
                    char ** ptr)
                    : buffer_(ptr)
                {
                }

                ~SymbolBuffer()
                {
                    if (buffer_) {
                        free(buffer_);
                    }
                }

                void next()
                {
                    push('\0');
                    *addrs_++ = (char *)(buf_ - (char *)buffer_);
                }

                void push(
                    char const * str)
                {
                    push(str, strlen(str));
                }

                void push(
                    char chr)
                {
                    push(&chr, 1);
                }

                void push(
                    size_t num)
                {
                    char buf[16];
                    char * p = buf;
                    while (num) {
                        *p++ = '0' + num % 10;
                        num /= 10;
                    }
                    prepare(p - buf);
                    for (--p; p >= buf; --p) {
                        *buf_++ = *p;
                    }
                    left_ -= p - buf;
                }

                void push_hex(
                    void const * addr, 
                    size_t size, 
                    char delim)
                {
                    char const * hex_chr = "0123456789abcdef";
                    prepare(size * 2 + delim ? size : 0);
                    char const * data = (char const *)(addr);
                    for (size_t i = 0; i < size; i++) {
                        char d = data[i];
                        *buf_++ = hex_chr[(d >> 4) & 0x0F];
                        *buf_++ = hex_chr[d & 0x0F];
                        if (delim) {
                            *buf_++ = delim;
                        }
                    }
                    left_ -= size * 2 + delim ? size : 0;
                }

                char ** detach()
                {
                    push('\0');
                    addrs_ = (char **)buffer_;
                    for (size_t i = 0; i < num_; ++i) {
                        addrs_[i] = (char *)buffer_ + size_t(addrs_[i]);
                    }
                    addrs_ = (char **)realloc(buffer_, buf_ - (char *)buffer_);
                    buffer_ = NULL;
                    return addrs_;
                }

            private:
                void push(
                    char const * ptr, 
                    size_t size)
                {
                    prepare(size);
                    memcpy(buf_, ptr, size);
                    buf_ += size;
                    left_ -= size;
                }

                void prepare(
                    size_t size)
                {
                    if (size > left_) {
                        while (size > left_) {
                            size_ += 1024;
                            left_ += 1024;
                        }
                        size_t num = addrs_ - (char **)buffer_;
                        buffer_ = realloc(buffer_, size_);
                        addrs_ = (char **)buffer_ + num;
                        buf_ = (char *)buffer_ + size_ - left_;
                    }
                }

            private:
                size_t num_;
                size_t size_;
                void * buffer_;
                char ** addrs_;
                char * buf_;
                size_t left_;
            };

        } // namespace detail

    } // namespace debuging
} // namespace framework
