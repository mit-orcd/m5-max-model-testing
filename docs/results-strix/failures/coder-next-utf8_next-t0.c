#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s == NULL || cp == NULL) {
        return -1;
    }

    unsigned char c = s[0];
    size_t len;

    if (c < 0x80) {
        *cp = c;
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        len = 2;
    } else if ((c & 0xF0) == 0xE0) {
        len = 3;
    } else if ((c & 0xF8) == 0xF0) {
        len = 4;
    } else {
        return -1;
    }

    if (len > n) {
        return -1;
    }

    uint32_t codepoint = c & ((1 << (8 - len)) - 1);
    for (size_t i = 1; i < len; ++i) {
        unsigned char byte = s[i];
        if ((byte & 0xC0) != 0x80) {
            return -1;
        }
        codepoint = (codepoint << 6) | (byte & 0x3F);
    }

    // Check for overlong encoding
    if (len == 2 && codepoint < 0x80) {
        return -1;
    }
    if (len == 3 && codepoint < 0x800) {
        return -1;
    }
    if (len == 4 && codepoint < 0x10000) {
        return -1;
    }

    // Check for surrogates and out-of-range
    if ((codepoint >= 0xD800 && codepoint <= 0xDFFF) || codepoint > 0x10FFFF) {
        return -1;
    }

    *cp = codepoint;
    return (int)len;
}