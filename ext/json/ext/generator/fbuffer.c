#include "ruby.h"
#include "fbuffer.h"

inline FBuffer *fbuffer_alloc()
{
    FBuffer *fb = ALLOC(FBuffer);
    memset((void *) fb, 0, sizeof(FBuffer));
    return fb;
}

inline void fbuffer_free(FBuffer *fb)
{
    if (fb->ptr) ruby_xfree(fb->ptr);
    ruby_xfree(fb);
}

inline void fbuffer_inc_capa(FBuffer *fb, unsigned int requested)
{
    unsigned int required;

    if (!fb->ptr) {
        fb->ptr = ALLOC_N(char, FBUFFER_INITIAL_LENGTH);
        fb->capa = FBUFFER_INITIAL_LENGTH;
    }

    for (required = fb->capa; requested > required - fb->len; required <<= 1);

    if (required > fb->capa) {
        fb->ptr = (char *) REALLOC_N((long*) fb->ptr, char, required);
        fb->capa = required;
    }
}

inline void fbuffer_append(FBuffer *fb, const char *newstr, unsigned int len)
{
    if (len > 0) {
        fbuffer_inc_capa(fb, len);
        memcpy(fb->ptr + fb->len, newstr, len);
        fb->len += len;
    }
}

inline void fbuffer_append_char(FBuffer *fb, const char newchr)
{
    fbuffer_inc_capa(fb, 1);
    *(fb->ptr + fb->len) = newchr;
    fb->len++;
}

