
#ifndef _PARSER_UNICODE_H_
#define _PARSER_UNICODE_H_

#include "ruby.h"

typedef unsigned long	UTF32;	/* at least 32 bits */
typedef unsigned short	UTF16;	/* at least 16 bits */
typedef unsigned char	UTF8;	/* typically 8 bits */

#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF

inline UTF32 unescape_unicode(const unsigned char *p);
inline int convert_UTF32_to_UTF8(char *buf, UTF32 ch);

#endif
