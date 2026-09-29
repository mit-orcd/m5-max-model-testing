#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    unsigned char b0 = s[0];
    if (b0 < 0x80) {
        if (n < 1) return -1;
        *cp = b0;
        return 1;
    } else if (b0 == 0xC0 || b0 == 0xC1) {
        return -1;
    } else if (b0 >= 0xE0 && b0 < 0xF0) {
        if (n < 3) return -1;
        unsigned char b1 = s[1], b2 = s[2];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) return -1;
        uint32_t codepoint = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
        if (codepoint < 0x800) return -1;
        *cp = codepoint;
        return 3;
    } else if (b0 >= 0xF0 && b0 < 0xF8) {
        if (n < 4) return -1;
        unsigned char b1 = s[1], b2 = s[2], b3 = s[3];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) return -1;
        uint32_t codepoint = ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        if (codepoint < 0x10000 || codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) return -1;
        *cp = codepoint;
        return 4;
    } else {
        return -1;
    }
}