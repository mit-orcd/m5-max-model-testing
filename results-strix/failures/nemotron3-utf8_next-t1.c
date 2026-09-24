#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c0 = s[0];
    if (c0 < 0x80) {
        if (n < 1) return -1;
        *cp = c0;
        return 1;
    }

    if ((c0 & 0xE0) == 0xC0) { // 2-byte
        if (n < 2) return -1;
        unsigned char c1 = s[1];
        if ((c1 & 0xC0) != 0x80) return -1;
        uint32_t code = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
        if (code < 0x80) return -1; // overlong
        *cp = code;
        return 2;
    }

    if ((c0 & 0xF0) == 0xE0) { // 3-byte
        if (n < 3) return -1;
        unsigned char c1 = s[1];
        unsigned char c2 = s[2];
        if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80) return -1;
        uint32_t code = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
        if (code < 0x800) return -1; // overlong
        if (code >= 0xD800 && code <= 0xDFFF) return -1; // surrogate
        *cp = code;
        return 3;
    }

    if ((c0 & 0xF8) == 0xF0) { // 4-byte
        if (n < 4) return -1;
        unsigned char c1 = s[1];
        unsigned char c2 = s[2];
        unsigned char c3 = s[3];
        if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) return -1;
        uint32_t code = ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        if (code < 0x10000) return -1; // overlong
        if (code > 0x10FFFF) return -1; // beyond Unicode
        *cp = code;
        return 4;
    }

    return -1; // invalid leading byte (continuation or 5/6-byte)
}