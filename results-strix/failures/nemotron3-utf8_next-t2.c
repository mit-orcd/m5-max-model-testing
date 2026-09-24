#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c0 = s[0];
    int len = 0;
    uint32_t codepoint = 0;

    if ((c0 & 0x80) == 0x00) { // 1-byte: 0xxxxxxx
        len = 1;
        codepoint = c0;
    } else if ((c0 & 0xE0) == 0xC0) { // 2-byte: 110xxxxx 10xxxxxx
        len = 2;
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        codepoint = ((c0 & 0x1F) << 6) | (s[1] & 0x3F);
        if (codepoint < 0x80) return -1; // overlong
    } else if ((c0 & 0xF0) == 0xE0) { // 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
        len = 3;
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        codepoint = ((c0 & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        if (codepoint < 0x800) return -1; // overlong
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1; // surrogate
    } else if ((c0 & 0xF8) == 0xF0) { // 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        len = 4;
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        codepoint = ((c0 & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        if (codepoint < 0x10000) return -1; // overlong
        if (codepoint > 0x10FFFF) return -1; // beyond Unicode
    } else {
        return -1; // invalid leading byte (continuation or 5/6-byte)
    }

    *cp = codepoint;
    return len;
}