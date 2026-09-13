#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c0 = s[0];
    int len;
    uint32_t codepoint;

    if ((c0 & 0x80) == 0) {
        // 1-byte: 0xxxxxxx
        len = 1;
        codepoint = c0;
    } else if ((c0 & 0xE0) == 0xC0) {
        // 2-byte: 110xxxxx
        if (n < 2) return -1;
        unsigned char c1 = s[1];
        if ((c1 & 0xC0) != 0x80) return -1;
        codepoint = ((c0 & 0x1F) << 6) | (c1 & 0x3F);
        // Check overlong (U+0000 to U+007F must be 1 byte)
        if (codepoint < 0x80) return -1;
        len = 2;
    } else if ((c0 & 0xF0) == 0xE0) {
        // 3-byte: 1110xxxx
        if (n < 3) return -1;
        unsigned char c1 = s[1];
        unsigned char c2 = s[2];
        if ((c1 & 0xC0) != 0x80) return -1;
        if ((c2 & 0xC0) != 0x80) return -1;
        codepoint = ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
        // Check overlong (U+0080 to U+07FF must be 2 bytes)
        if (codepoint < 0x800) return -1;
        // Check surrogate range
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
        // Check max valid
        if (codepoint > 0xFFFF) return -1;
        len = 3;
    } else if ((c0 & 0xF8) == 0xF0) {
        // 4-byte: 11110xxx
        if (n < 4) return -1;
        unsigned char c1 = s[1];
        unsigned char c2 = s[2];
        unsigned char c3 = s[3];
        if ((c1 & 0xC0) != 0x80) return -1;
        if ((c2 & 0xC0) != 0x80) return -1;
        if ((c3 & 0xC0) != 0x80) return -1;
        codepoint = ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        // Check overlong (U+0800 to U+FFFF must be 3 bytes)
        if (codepoint < 0x10000) return -1;
        // Check max valid (U+10FFFF)
        if (codepoint > 0x10FFFF) return -1;
        len = 4;
    } else {
        // Invalid leading byte (10xxxxxx or 11111xxx+)
        return -1;
    }

    *cp = codepoint;
    return len;
}