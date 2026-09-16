#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c0 = s[0];
    int len = 0;
    uint32_t code = 0;

    if (c0 < 0x80) {
        len = 1;
        code = c0;
    } else if ((c0 & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        len = 2;
        code = ((c0 & 0x1F) << 6) | (s[1] & 0x3F);
        if ((s[1] & 0xC0) != 0x80) return -1;
        if (code < 0x80) return -1; // overlong
    } else if ((c0 & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        len = 3;
        code = ((c0 & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        if (code < 0x800) return -1; // overlong
        if (code >= 0xD800 && code <= 0xDFFF) return -1; // surrogates
    } else if ((c0 & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        len = 4;
        code = ((c0 & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        if (code < 0x10000) return -1; // overlong
        if (code > 0x10FFFF) return -1; // beyond Unicode
    } else {
        return -1; // invalid leading byte (including 5/6-byte and continuation)
    }

    *cp = code;
    return len;
}