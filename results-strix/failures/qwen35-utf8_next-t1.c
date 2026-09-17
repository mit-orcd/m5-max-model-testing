#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c = s[0];
    int len;
    uint32_t codepoint;

    if ((c & 0x80) == 0) {
        // 1-byte sequence: 0xxxxxxx
        len = 1;
        codepoint = c;
    } else if ((c & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        len = 2;
        codepoint = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        // Check for overlong encoding (must be >= 0x80)
        if (codepoint < 0x80) return -1;
        // Check for surrogate or out of range
        if (codepoint > 0x7FF) return -1;
    } else if ((c & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        len = 3;
        codepoint = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        // Check for overlong encoding (must be >= 0x800)
        if (codepoint < 0x800) return -1;
        // Check for surrogates (U+D800 to U+DFFF)
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
        // Check upper bound
        if (codepoint > 0xFFFF) return -1;
    } else if ((c & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        len = 4;
        codepoint = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        // Check for overlong encoding (must be >= 0x10000)
        if (codepoint < 0x10000) return -1;
        // Check upper bound (max valid UTF-8 is U+10FFFF)
        if (codepoint > 0x10FFFF) return -1;
    } else {
        // Invalid leading byte (continuation byte, reserved, or too long form)
        return -1;
    }

    *cp = codepoint;
    return len;
}