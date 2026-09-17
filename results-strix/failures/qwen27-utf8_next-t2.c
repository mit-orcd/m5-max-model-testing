#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char b0 = s[0];

    /* Leading byte must not be a continuation byte (10xxxxxx) */
    if ((b0 & 0xC0) == 0x80) return -1;

    /* 1-byte: 0xxxxxxx */
    if ((b0 & 0x80) == 0) {
        *cp = (uint32_t)b0;
        return 1;
    }

    /* 2-byte: 110xxxxx */
    if ((b0 & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        unsigned char b1 = s[1];
        if ((b1 & 0xC0) != 0x80) return -1;
        uint32_t c = ((uint32_t)(b0 & 0x1F) << 6) | (uint32_t)(b1 & 0x3F);
        /* Overlong: must be >= 0x80 */
        if (c < 0x80) return -1;
        *cp = c;
        return 2;
    }

    /* 3-byte: 1110xxxx */
    if ((b0 & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        unsigned char b1 = s[1];
        unsigned char b2 = s[2];
        if ((b1 & 0xC0) != 0x80) return -1;
        if ((b2 & 0xC0) != 0x80) return -1;
        uint32_t c = ((uint32_t)(b0 & 0x0F) << 12) |
                     ((uint32_t)(b1 & 0x3F) << 6) |
                     (uint32_t)(b2 & 0x3F);
        /* Overlong: must be >= 0x800 */
        if (c < 0x800) return -1;
        /* Surrogates: U+D800..U+DFFF */
        if (c >= 0xD800 && c <= 0xDFFF) return -1;
        *cp = c;
        return 3;
    }

    /* 4-byte: 11110xxx */
    if ((b0 & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        unsigned char b1 = s[1];
        unsigned char b2 = s[2];
        unsigned char b3 = s[3];
        if ((b1 & 0xC0) != 0x80) return -1;
        if ((b2 & 0xC0) != 0x80) return -1;
        if ((b3 & 0xC0) != 0x80) return -1;
        uint32_t c = ((uint32_t)(b0 & 0x07) << 18) |
                     ((uint32_t)(b1 & 0x3F) << 12) |
                     ((uint32_t)(b2 & 0x3F) << 6) |
                     (uint32_t)(b3 & 0x3F);
        /* Overlong: must be >= 0x10000 */
        if (c < 0x10000) return -1;
        /* Above max: must be <= 0x10FFFF */
        if (c > 0x10FFFF) return -1;
        *cp = c;
        return 4;
    }

    /* 5-byte (111110xx) and 6-byte (1111110x) forms are obsolete/invalid */
    return -1;
}