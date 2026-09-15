#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0)
        return -1;

    unsigned char b0 = s[0];

    if (b0 <= 0x7F) {
        if (cp) *cp = b0;
        return 1;
    }

    if ((b0 & 0xE0) == 0xC0) {
        if (n < 2)
            return -1;
        unsigned char b1 = s[1];
        if ((b1 & 0xC0) != 0x80)
            return -1;
        uint32_t c = ((uint32_t)(b0 & 0x1F) << 6) | (b1 & 0x3F);
        if (c < 0x80)
            return -1;
        if (cp) *cp = c;
        return 2;
    }

    if ((b0 & 0xF0) == 0xE0) {
        if (n < 3)
            return -1;
        unsigned char b1 = s[1], b2 = s[2];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80)
            return -1;
        uint32_t c = ((uint32_t)(b0 & 0x0F) << 12) |
                     ((uint32_t)(b1 & 0x3F) << 6) |
                     (b2 & 0x3F);
        if (c < 0x800)
            return -1;
        if (c >= 0xD800 && c <= 0xDFFF)
            return -1;
        if (cp) *cp = c;
        return 3;
    }

    if ((b0 & 0xF8) == 0xF0) {
        if (n < 4)
            return -1;
        unsigned char b1 = s[1], b2 = s[2], b3 = s[3];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80)
            return -1;
        uint32_t c = ((uint32_t)(b0 & 0x07) << 18) |
                     ((uint32_t)(b1 & 0x3F) << 12) |
                     ((uint32_t)(b2 & 0x3F) << 6) |
                     (b3 & 0x3F);
        if (c < 0x10000)
            return -1;
        if (c > 0x10FFFF)
            return -1;
        if (cp) *cp = c;
        return 4;
    }

    return -1;
}