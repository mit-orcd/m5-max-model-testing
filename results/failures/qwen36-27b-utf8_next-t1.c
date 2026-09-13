#include <stdint.h>
#include <stddef.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c = s[0];
    uint32_t codepoint;
    int len;

    if ((c & 0x80) == 0x00) {
        // 1-byte sequence: 0xxxxxxx
        len = 1;
        codepoint = c;
        if (codepoint == 0) return 1; // Allow NUL? Usually yes, but check overlong. 0 is not overlong.
        // U+0000 is valid.
    } else if ((c & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        codepoint = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        len = 2;
        if (codepoint < 0x80) return -1; // Overlong
    } else if ((c & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        codepoint = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        len = 3;
        if (codepoint < 0x800) return -1; // Overlong
    } else if ((c & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        codepoint = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        len = 4;
        if (codepoint < 0x10000) return -1; // Overlong
        if (codepoint > 0x10FFFF) return -1; // Out of range
    } else {
        // Invalid leading byte (continuation byte or 5/6 byte start)
        return -1;
    }

    // Reject surrogates
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;

    *cp = codepoint;
    return len;
}