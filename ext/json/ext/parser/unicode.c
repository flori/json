#include "unicode.h"

/*
 * Copyright 2001-2004 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

static const char digit_values[256] = { 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1,
    -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1
};

char *JSON_convert_UTF16_to_UTF8 (
        VALUE buffer,
        char *source,
        char *sourceEnd)
{
    UTF16 *tmp, *tmpPtr, *tmpEnd;
    char buf[5];
    long n = 0;
    char failed = 1, c, *p = source - 1;

    while (p < sourceEnd && p[0] == '\\' && p[1] == 'u') {
        p += 6;
        n++;
    }
    p = source + 1;
    tmpPtr = tmp = ALLOC_N(UTF16, n);
    tmpEnd = tmp + n;
    while (tmpPtr < tmpEnd) {
        c = digit_values[(unsigned char) *p++];
        failed *= c;
        *tmpPtr = c << 12;
        c = digit_values[(unsigned char) *p++];
        failed *= c;
        *tmpPtr |= c << 8;
        c = digit_values[(unsigned char) *p++];
        failed *= c;
        *tmpPtr |= c << 4;
        c = digit_values[(unsigned char) *p++];
        failed *= c;
        *tmpPtr++ |= c;
        p += 2;
    }
    if (failed < 0) {
        rb_raise(rb_path2class("JSON::ParserError"),
                "illegal \\uXXXX unicode value near %s", source);
    }

    tmpPtr = tmp;
    while (tmpPtr < tmpEnd) {
        UTF32 ch;
        unsigned short bytesToWrite = 0;
        const UTF32 byteMask = 0xBF;
        const UTF32 byteMark = 0x80; 
        ch = *tmpPtr++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
            /* If the 16 bits following the high surrogate are in the source
             * buffer... */
            if (tmpPtr < tmpEnd) {
                UTF32 ch2 = *tmpPtr;
                /* If it's a low surrogate, convert to UTF32. */
                if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                    ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
                        + (ch2 - UNI_SUR_LOW_START) + halfBase;
                    ++tmpPtr;
                }
            } else { /* We don't have the 16 bits following the high surrogate. */
                ruby_xfree(tmp);
                rb_raise(rb_path2class("JSON::ParserError"),
                    "partial character in source, but hit end near %s", source);
                break;
            }
        }
        /* Figure out how many bytes the result will require */
        if (ch < (UTF32) 0x80) {
            bytesToWrite = 1;
        } else if (ch < (UTF32) 0x800) {
            bytesToWrite = 2;
        } else if (ch < (UTF32) 0x10000) {
            bytesToWrite = 3;
        } else if (ch < (UTF32) 0x110000) {
            bytesToWrite = 4;
        } else {
            bytesToWrite = 3;
            ch = UNI_REPLACEMENT_CHAR;
        }

        buf[0] = 0;
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = 0;
        p = buf + bytesToWrite;
        switch (bytesToWrite) { /* note: everything falls through. */
            case 4: *--p = (UTF8) ((ch | byteMark) & byteMask); ch >>= 6;
            case 3: *--p = (UTF8) ((ch | byteMark) & byteMask); ch >>= 6;
            case 2: *--p = (UTF8) ((ch | byteMark) & byteMask); ch >>= 6;
            case 1: *--p = (UTF8) (ch | firstByteMark[bytesToWrite]);
        }
        rb_str_buf_cat(buffer, p, bytesToWrite);
    }
    ruby_xfree(tmp);
    source += 6 * n - 1;
    return source;
}
