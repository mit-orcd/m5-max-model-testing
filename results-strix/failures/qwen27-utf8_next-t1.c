#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp)
{
    if (n == 0) return -1;

    unsigned char c0 = s[0];

    /* Reject continuation byte in leading position */
    if ((c0 & 0xC0) == 0x80) return -1;

    /* 1-byte: 0xxxxxxx */
    if (c0 < 0x80) {
        *cp = (uint32_t)c0;
        return 1;
    }

    /* 2-byte: 110xxxxx 10xxxxxx */
    if ((c0 & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        unsigned char c1 = s[1];
        if ((c1 & 0xC0) != 0x80) return -1;
        uint32_t code = ((uint32_t)(c0 & 0x1F) << 6) | (uint32_t)(c1 & 0x3F);
        /* Reject overlong: code point must be >= 0x80 */
        if (code < 0x80) return -1;
        *cp = code;
        return 2;
    }

    /* 3-byte: 1110xxxx 10xxxxxx 10xxxxxx */
    if ((c0 & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        unsigned char c1 = s[1];
        unsigned char c2 = s[2];
        if ((c1 & 0xC0) != 0x80) return -1;
        if ((c2 & 0xC0) != 0x80) return -1;
        uint32_t code = ((uint32_t)(c0 & 0x0F) << 12)
                      | ((uint32_t)(c1 & 0x3F) << 6)
                      |  (uint32_t)(c2 & 0x3F);
        /* Reject overlong: code point must be >= 0x800 */
        if (code < 0x800) return -1;
        /* Reject surrogates */
        if (code >= 0xD800 && code <= 0xDFFF) return -1;
        *cp = code;
        return 3;
    }

    /* 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if ((c0 & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        unsigned char c1 = s[1];
        unsigned char c2 = s[2];
        unsigned char c3 = s[3];
        if ((c1 & 0xC0) != 0x80) return -1;
        if ((c2 & 0xC0) != 0x80) return -1;
        if ((c3 & 0xC0) != 0x80) return -1;
        uint32_t code = ((uint32_t)(c0 & 0x07) << 18)
                      | ((uint32_t)(c1 & 0x3F) << 12)
                      | ((uint32_t)(c2 & 0x3F) << 6)
                      |  (uint32_t)(c3 & 0x3F);
        /* Reject overlong: code point must be >= 0x10000 */
        if (code < 0x10000) return -1;
        /* Reject anything above U+10FFFF */
        if (code > 0x10FFFF) return -1;
        *cp = code;
        return 4;
    }

    /* 5-byte, 6-byte forms, and any other invalid leading byte */
    return -1;
}