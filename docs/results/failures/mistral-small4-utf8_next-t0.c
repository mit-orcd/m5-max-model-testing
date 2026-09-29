#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s == NULL) return -1;

    uint32_t codepoint = 0;
    int length = 1;
    unsigned char c = s[0];

    if (c <= 0x7F) {
        codepoint = c;
    } else if ((c & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        codepoint = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        if (codepoint < 0x80) return -1; // overlong encoding
        length = 2;
    } else if ((c & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        codepoint = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        if (codepoint < 0x800) return -1; // overlong encoding
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1; // surrogate
        if (codepoint > 0x10FFFF) return -1; // beyond Unicode range
        length = 3;
    } else if ((c & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        codepoint = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        if (codepoint < 0x10000) return -1; // overlong encoding
        if (codepoint > 0x10FFFF) return -1; // beyond Unicode range
        length = 4;
    } else {
        return -1; // bad leading byte
    }

    *cp = codepoint;
    return length;
}