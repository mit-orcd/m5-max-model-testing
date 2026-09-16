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
        len = 2;
        code = c0 & 0x1F;
    } else if ((c0 & 0xF0) == 0xE0) {
        len = 3;
        code = c0 & 0x0F;
    } else if ((c0 & 0xF8) == 0xF0) {
        len = 4;
        code = c0 & 0x07;
    } else {
        return -1; // Invalid leading byte (including 5/6-byte, continuation)
    }

    if (len > (int)n) return -1; // Truncated

    for (int i = 1; i < len; i++) {
        unsigned char c = s[i];
        if ((c & 0xC0) != 0x80) return -1; // Not a continuation byte
        code = (code << 6) | (c & 0x3F);
    }

    // Check for overlong encoding
    if (len == 2 && code < 0x80) return -1;
    if (len == 3 && code < 0x800) return -1;
    if (len == 4 && code < 0x10000) return -1;

    // Check for surrogate range
    if (code >= 0xD800 && code <= 0xDFFF) return -1;

    // Check for code point > U+10FFFF
    if (code > 0x10FFFF) return -1;

    *cp = code;
    return len;
}