#include <stdint.h>
#include <stddef.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp)
{
    if (n == 0) return -1;
    unsigned char b1 = s[0];

    /* 1-byte ASCII */
    if (b1 <= 0x7F) {
        *cp = b1;
        return 1;
    }

    /* 2-byte sequence */
    if (b1 >= 0xC2 && b1 <= 0xDF) {
        if (n < 2) return -1;
        unsigned char b2 = s[1];
        if (b2 < 0x80 || b2 > 0xBF) return -1;
        *cp = ((uint32_t)(b1 & 0x1F) << 6) | (b2 & 0x3F);
        return 2;
    }

    /* 3-byte sequence */
    if (b1 == 0xE0) {
        if (n < 3) return -1;
        unsigned char b2 = s[1], b3 = s[2];
        if (b2 < 0xA0 || b2 > 0xBF) return -1;
        if (b3 < 0x80 || b3 > 0xBF) return -1;
        *cp = ((uint32_t)(b1 & 0x0F) << 12) |
              ((uint32_t)(b2 & 0x3F) << 6) |
              (b3 & 0x3F);
        return 3;
    }
    if ((b1 >= 0xE1 && b1 <= 0xEC) ||
        (b1 >= 0xEE && b1 <= 0xEF)) {
        if (n < 3) return -1;
        unsigned char b2 = s[1], b3 = s[2];
        if (b2 < 0x80 || b2 > 0xBF) return -1;
        if (b3 < 0x80 || b3 > 0xBF) return -1;
        *cp = ((uint32_t)(b1 & 0x0F) << 12) |
              ((uint32_t)(b2 & 0x3F) << 6) |
              (b3 & 0x3F);
        return 3;
    }
    if (b1 == 0xED) {
        if (n < 3) return -1;
        unsigned char b2 = s[1], b3 = s[2];
        if (b2 < 0x80 || b2 > 0x9F) return -1;   /* surrogate start */
        if (b3 < 0x80 || b3 > 0xBF) return -1;
        /* Surrogate range U+D800-U+DFFF is invalid */
        return -1;
    }

    /* 4-byte sequence */
    if (b1 == 0xF0) {
        if (n < 4) return -1;
        unsigned char b2 = s[1], b3 = s[2], b4 = s[3];
        if (b2 < 0x90 || b2 > 0xBF) return -1;
        if (b3 < 0x80 || b3 > 0xBF) return -1;
        if (b4 < 0x80 || b4 > 0xBF) return -1;
        *cp = ((uint32_t)(b1 & 0x07) << 18) |
              ((uint32_t)(b2 & 0x3F) << 12) |
              ((uint32_t)(b3 & 0x3F) << 6) |
              (b4 & 0x3F);
        return 4;
    }
    if (b1 >= 0xF1 && b1 <= 0xF3) {
        if (n < 4) return -1;
        unsigned char b2 = s[1], b3 = s[2], b4 = s[3];
        if (b2 < 0x80 || b2 > 0xBF) return -1;
        if (b3 < 0x80 || b3 > 0xBF) return -1;
        if (b4 < 0x80 || b4 > 0xBF) return -1;
        *cp = ((uint32_t)(b1 & 0x07) << 18) |
              ((uint32_t)(b2 & 0x3F) << 12) |
              ((uint32_t)(b3 & 0x3F) << 6) |
              (b4 & 0x3F);
        return 4;
    }
    if (b1 == 0xF4) {
        if (n < 4) return -1;
        unsigned char b2 = s[1], b3 = s[2], b4 = s[3];
        if (b2 < 0x80 || b2 > 0x8F) return -1;   /* limit to U+10FFFF */
        if (b3 < 0x80 || b3 > 0xBF) return -1;
        if (b4 < 0x80 || b4 > 0xBF) return -1;
        *cp = ((uint32_t)(b1 & 0x07) << 18) |
              ((uint32_t)(b2 & 0x3F) << 12) |
              ((uint32_t)(b3 & 0x3F) << 6) |
              (b4 & 0x3F);
        return 4;
    }

    /* All other leading bytes are invalid */
    return -1;
}