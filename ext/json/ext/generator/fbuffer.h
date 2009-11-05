#ifndef _FBUFFER_H_
#define _FBUFFER_H_

#include <assert.h>
#include "ruby.h"

typedef struct FBufferStruct {
    unsigned int initial_length;
    char *ptr;
    unsigned int len;
    unsigned int capa;
} FBuffer;

#define FBUFFER_INITIAL_LENGTH 4096

#define FBUFFER_PTR(fb) (fb->ptr)
#define FBUFFER_LEN(fb) (fb->len)
#define FBUFFER_CAPA(fb) (fb->capa)
#define FBUFFER_PAIR(fb) FBUFFER_PTR(fb), FBUFFER_LEN(fb)

inline FBuffer *fbuffer_alloc();
inline FBuffer *fbuffer_alloc_with_length(unsigned initial_length);
inline void fbuffer_free(FBuffer *fb);
inline void fbuffer_free_only_buffer(FBuffer *fb);
inline void fbuffer_clear(FBuffer *fb);
inline void fbuffer_append(FBuffer *fb, const char *newstr, unsigned int len);
inline void fbuffer_append_char(FBuffer *fb, char newchr);

#endif
