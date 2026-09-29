#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c = s[0];
    if ((c & 0x80) == 0x00) {
        if (cp) *cp = c;
        return 1;
    }

    if ((c & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        uint32_t code = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        if (code < 0x80) return -1; // Overlong
        if (cp) *cp = code;
        return 2;
    }

    if ((c & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        uint32_t code = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        if (code < 0x800) return -1; // Overlong
        if (code >= 0xD800 && code <= 0xDFFF) return -1; // Surrogate
        if (cp) *cp = code;
        return 3;
    }

    if ((c & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        uint32_t code = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        if (code < 0x10000) return -1; // Overlong
        if (code > 0x10FFFF) return -1; // Beyond Unicode
        if (cp) *cp = code;
        return 4;
    }

    return -1;
}