#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || cp == NULL) {
        return -1;
    }

    unsigned char c = s[0];
    uint32_t codepoint;
    size_t len;

    if (c < 0x80) {
        // 1-byte sequence: 0xxxxxxx
        codepoint = c;
        len = 1;
    } else if ((c & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        if (n < 2 || (s[1] & 0xC0) != 0x80) {
            return -1;
        }
        codepoint = (c & 0x1F) << 6 | (s[1] & 0x3F);
        len = 2;
        // Reject overlong encodings (codepoint < 0x80)
        if (codepoint < 0x80) {
            return -1;
        }
    } else if ((c & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        if (n < 3 || (s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) {
            return -1;
        }
        codepoint = (c & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
        len = 3;
        // Reject overlong encodings and surrogates (codepoint < 0x800 or in surrogate range)
        if (codepoint < 0x800 || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
            return -1;
        }
    } else if ((c & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if (n < 4 || (s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) {
            return -1;
        }
        codepoint = (c & 0x07) << 18 | (s[1] & 0x3F) << 12 | (s[2] & 0x3F) << 6 | (s[3] & 0x3F);
        len = 4;
        // Reject overlong encodings and codepoints > 0x10FFFF
        if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
            return -1;
        }
    } else {
        // Invalid leading byte (e.g., continuation byte or obsolete 5-/6-byte form)
        return -1;
    }

    *cp = codepoint;
    return (int)len;
}