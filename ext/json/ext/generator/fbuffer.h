typedef struct FBufferStruct {
    unsigned char *ptr;
    unsigned int len;
    unsigned int capa;
} FBuffer;

#define FBUFFER_INITIAL_LENGTH 4096

#define FBUFFER_PTR(fb) (fb->ptr)

#define FBUFFER_LEN(fb) (fb->len)

FBuffer *fbuffer_alloc();
void fbuffer_free(FBuffer *fb);
inline void fbuffer_inc_capa(FBuffer *fb, unsigned int requested);
inline void fbuffer_append(FBuffer *fb, const unsigned char *newstr, unsigned int len);
inline void fbuffer_append_char(FBuffer *fb, const unsigned char newchr);
