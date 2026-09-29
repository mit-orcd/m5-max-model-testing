#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s == 0 || cp == 0) return -1;

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

    if (len > (int)n) return -1;

    for (int i = 1; i < len; ++i) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        codepoint = (codepoint << 6) | (s[i] & 0x3F);
    }

    /* Check for overlong encodings */
    switch (len) {
        case 2:
            if (codepoint < 0x80) return -1;
            break;
        case 3:
            if (codepoint < 0x800) return -1;
            if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
            break;
        case 4:
            if (codepoint < 0x10000) return -1;
            if (codepoint > 0x10FFFF) return -1;
            break;
    }

    *cp = codepoint;
    return len;
}