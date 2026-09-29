#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c = s[0];
    int len;

    if (c < 0x80) {
        len = 1;
    } else if ((c & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        len = 2;
    } else if ((c & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        len = 3;
    } else if ((c & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        len = 4;
    } else {
        return -1;
    }

    uint32_t codepoint;
    switch (len) {
        case 1:
            codepoint = c;
            break;
        case 2:
            if ((c & 0x1E) == 0) return -1; // Overlong
            codepoint = ((c & 0x1F) << 6) | (s[1] & 0x3F);
            break;
        case 3:
            if ((c & 0x0F) == 0) return -1; // Overlong
            codepoint = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
            if (codepoint <= 0x7FF) return -1; // Overlong
            break;
        case 4:
            if ((c & 0x07) == 0) return -1; // Overlong
            codepoint = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
            if (codepoint <= 0xFFFF) return -1; // Overlong
            if (codepoint > 0x10FFFF) return -1; // Beyond Unicode
            break;
    }

    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1; // Surrogate

    *cp = codepoint;
    return len;
}