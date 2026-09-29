#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n < 1) return -1;

    unsigned char c = s[0];
    uint32_t codepoint = 0;
    int bytes_read = 0;

    if (c < 0x80) {
        // 1-byte sequence: 0xxxxxxx
        if (n < 1) return -1;
        codepoint = c;
        bytes_read = 1;
    } else if ((c & 0xe0) == 0xc0) {
        // 2-byte sequence: 110xxxxx
        if (n < 2) return -1;
        if ((s[1] & 0xc0) != 0x80) return -1;
        codepoint = ((c & 0x1f) << 6) | (s[1] & 0x3f);
        // Overlong check: must be at least 128 (0x80)
        if (codepoint < 0x80) return -1;
        bytes_read = 2;
    } else if ((c & 0xf0) == 0xe0) {
        // 3-byte sequence: 1110xxxx
        if (n < 3) return -1;
        if ((s[1] & 0xc0) != 0x80) return -1;
        if ((s[2] & 0xc0) != 0x80) return -1;
        codepoint = ((c & 0x0f) << 12) | ((s[1] & 0x3f) << 6) | (s[2] & 0x3f);
        // Overlong check: must be at least 0x800 (11100000 10xxxxxx 10xxxxxx)
        if (codepoint < 0x800) return -1;
        // Surrogate check: must not be in U+D800-U+DFFF
        if (codepoint >= 0xd800 && codepoint <= 0xdfff) return -1;
        bytes_read = 3;
    } else if ((c & 0xf8) == 0xf0) {
        // 4-byte sequence: 11110xxx
        if (n < 4) return -1;
        if ((s[1] & 0xc0) != 0x80) return -1;
        if ((s[2] & 0xc0) != 0x80) return -1;
        if ((s[3] & 0xc0) != 0x80) return -1;
        codepoint = ((c & 0x07) << 18) | ((s[1] & 0x3f) << 12) | ((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
        // Overlong check: must be at least 0x10000
        if (codepoint < 0x10000) return -1;
        // Range check: must be <= U+10FFFF
        if (codepoint > 0x10ffff) return -1;
        bytes_read = 4;
    } else {
        // Invalid start byte (10xxxxxx or 11111xxx...)
        return -1;
    }

    *cp = codepoint;
    return bytes_read;
}