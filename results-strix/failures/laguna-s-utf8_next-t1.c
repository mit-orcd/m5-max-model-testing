#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char b0 = s[0];

    /* 1-byte: 0xxxxxxx */
    if (b0 <= 0x7F) {
        *cp = b0;
        return 1;
    }

    /* 2-byte: 110xxxxx */
    if ((b0 & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        unsigned char b1 = s[1];
        if ((b1 & 0xC0) != 0x80) return -1;
        uint32_t cp_val = ((b0 & 0x1F) << 6) | (b1 & 0x3F);
        if (cp_val < 0x80) return -1; /* overlong */
        *cp = cp_val;
        return 2;
    }

    /* 3-byte: 1110xxxx */
    if ((b0 & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        unsigned char b1 = s[1];
        unsigned char b2 = s[2];
        if ((b1 & 0xC0) != 0x80) return -1;
        if ((b2 & 0xC0) != 0x80) return -1;
        uint32_t cp_val = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
        if (cp_val < 0x800) return -1; /* overlong */
        if (cp_val >= 0xD800 && cp_val <= 0xDFFF) return -1; /* surrogate */
        *cp = cp_val;
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
        uint32_t cp_val = ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) |
                          ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        if (cp_val < 0x10000) return -1; /* overlong */
        if (cp_val > 0x10FFFF) return -1; /* out of range */
        *cp = cp_val;
        return 4;
    }

    /* invalid leading byte or 5/6-byte sequences */
    return -1;
}