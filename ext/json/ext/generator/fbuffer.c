#include "fbuffer.h"

inline FBuffer *fbuffer_alloc()
{
    FBuffer *fb = ALLOC(FBuffer);
    memset((void *) fb, 0, sizeof(FBuffer));
    fb->initial_length = FBUFFER_INITIAL_LENGTH;
    return fb;
}

inline FBuffer *fbuffer_alloc_with_length(unsigned int initial_length)
{
    assert(initial_length > 0);
    FBuffer *fb = ALLOC(FBuffer);
    memset((void *) fb, 0, sizeof(FBuffer));
    fb->initial_length = initial_length;
    return fb;
}


inline void fbuffer_free(FBuffer *fb)
{
    if (fb->ptr) ruby_xfree(fb->ptr);
    ruby_xfree(fb);
}

inline void fbuffer_clear(FBuffer *fb)
{
    fb->len = 0;
}

static inline void fbuffer_inc_capa(FBuffer *fb, unsigned int requested)
{
    unsigned int required;

    if (!fb->ptr) {
        fb->ptr = ALLOC_N(char, fb->initial_length);
        fb->capa = fb->initial_length;
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

inline void fbuffer_append_char(FBuffer *fb, char newchr)
{
    fbuffer_inc_capa(fb, 1);
    *(fb->ptr + fb->len) = newchr;
    fb->len++;
}

