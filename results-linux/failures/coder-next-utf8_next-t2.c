#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) {
        return -1;
    }

    unsigned char c = s[0];
    int len;

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

    if (len > (int)n) {
        return -1;
    }

    if ((c & 0xC0) == 0x80) {
        return -1;
    }

    uint32_t codepoint = c & ((1 << (8 - len)) - 1);
    
    for (int i = 1; i < len; ++i) {
        unsigned char next = s[i];
        if ((next & 0xC0) != 0x80) {
            return -1;
        }
        codepoint = (codepoint << 6) | (next & 0x3F);
    }

    if (len == 2) {
        if (codepoint < 0x80 || codepoint > 0x7FF) {
            return -1;
        }
    } else if (len == 3) {
        if (codepoint < 0x800 || codepoint > 0xFFFF || 
            (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
            return -1;
        }
    } else if (len == 4) {
        if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
            return -1;
        }
    }

    *cp = codepoint;
    return len;
}