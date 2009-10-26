#ifndef _FBUFFER_H_
#define _FBUFFER_H_

typedef struct FBufferStruct {
    char *ptr;
    unsigned int len;
    unsigned int capa;
} FBuffer;

#define FBUFFER_INITIAL_LENGTH 4096

#define FBUFFER_PTR(fb) (fb->ptr)
#define FBUFFER_LEN(fb) (fb->len)
#define FBUFFER_PAIR(fb) FBUFFER_PTR(fb), FBUFFER_LEN(fb)

inline FBuffer *fbuffer_alloc();
inline void fbuffer_free(FBuffer *fb);
inline void fbuffer_clear(FBuffer *fb);
inline void fbuffer_inc_capa(FBuffer *fb, unsigned int requested);
inline void fbuffer_append(FBuffer *fb, const char *newstr, unsigned int len);
inline void fbuffer_append_char(FBuffer *fb, const char newchr);

#endif
