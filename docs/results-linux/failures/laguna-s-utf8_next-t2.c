#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char b0 = s[0];
    uint32_t codepoint;
    int len;

    if ((b0 & 0x80) == 0x00) {
        codepoint = b0;
        len = 1;
    } else if ((b0 & 0xE0) == 0xC0) {
        codepoint = (b0 & 0x1F);
        len = 2;
    } else if ((b0 & 0xF0) == 0xE0) {
        codepoint = (b0 & 0x0F);
        len = 3;
    } else if ((b0 & 0xF8) == 0xF0) {
        codepoint = (b0 & 0x07);
        len = 4;
    } else {
        return -1;
    }

    if (n < (size_t)len) return -1;

    for (int i = 1; i < len; i++) {
        unsigned char b = s[i];
        if ((b & 0xC0) != 0x80) return -1;
        codepoint = (codepoint << 6) | (b & 0x3F);
    }

    // Check for overlong encodings
    if (len == 2 && codepoint < 0x80) return -1;
    if (len == 3 && codepoint < 0x800) return -1;
    if (len == 4 && codepoint < 0x10000) return -1;

    // Check for surrogates
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;

    // Check for out of range
    if (codepoint > 0x10FFFF) return -1;

    *cp = codepoint;
    return len;
}