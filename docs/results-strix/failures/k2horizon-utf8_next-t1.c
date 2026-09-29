#include <stddef.h>
#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0)
        return -1;

    unsigned char c = s[0];

    if (c < 0x80) {
        *cp = c;
        return 1;
    }

    unsigned char b2, b3, b4;
    uint32_t cpval;
    int len;

    if (c >= 0xC2 && c <= 0xDF) {
        if (n < 2) return -1;
        b2 = s[1];
        if ((b2 & 0xC0) != 0x80) return -1;
        cpval = ((uint32_t)(c & 0x1F) << 6) | (b2 & 0x3F);
        len = 2;
    } else if (c == 0xE0) {
        if (n < 3) return -1;
        b2 = s[1];
        b3 = s[2];
        if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) return -1;
        if (b2 < 0xA0) return -1;  /* overlong */
        cpval = ((uint32_t)(b2 & 0x3F) << 12) | ((uint32_t)(b3 & 0x3F) << 6) | (b3 & 0x3F);
        len = 3;
    } else if (c >= 0xE1 && c <= 0xEC) {
        if (n < 3) return -1;
        b2 = s[1];
        b3 = s[2];
        if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) return -1;
        cpval = ((uint32_t)(c & 0x0F) << 12) | ((uint32_t)(b2 & 0x3F) << 6) | (b3 & 0x3F);
        len = 3;
    } else if (c == 0xED) {
        if (n < 3) return -1;
        b2 = s[1];
        b3 = s[2];
        if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) return -1;
        if (b2 > 0x9F) return -1;  /* surrogate */
        cpval = ((uint32_t)(b2 & 0x3F) << 12) | ((uint32_t)(b3 & 0x3F) << 6) | (b3 & 0x3F);
        len = 3;
    } else if (c >= 0xF0 && c <= 0xF4) {
        if (n < 4) return -1;
        b2 = s[1];
        b3 = s[2];
        b4 = s[3];
        if ((b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80 || (b4 & 0xC0) != 0x80) return -1;
        if (c == 0xF0 && b2 < 0x90) return -1;  /* overlong */
        if (c == 0xF4 && b2 > 0x8F) return -1;  /* > U+10FFFF */
        cpval = ((uint32_t)(c & 0x07) << 18) | ((uint32_t)(b2 & 0x3F) << 12) |
                ((uint32_t)(b3 & 0x3F) << 6) | (b4 & 0x3F);
        len = 4;
    } else {
        return -1;  /* lead byte 0x80-0xC1, 0xF5-0xFF, or 0xC0/C1 overlong */
    }

    *cp = cpval;
    return len;
}