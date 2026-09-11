#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || cp == NULL) {
        return -1;
    }

    unsigned char c = s[0];
    int len;
    uint32_t codepoint;

    // Determine length based on first byte
    if ((c & 0x80) == 0x00) {
        // 1-byte sequence: 0xxxxxxx
        len = 1;
        codepoint = c;
    } else if ((c & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        if (n < 2) return -1;
        if ((c & 0xFE) == 0xC0) return -1; // overlong
        len = 2;
        codepoint = c & 0x1F;
    } else if ((c & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        if (n < 3) return -1;
        if ((c == 0xED && (s[1] & 0xE0) == 0xA0) ||
            (c == 0xE0 && (s[1] & 0xF0) == 0x80)) {
            return -1; // overlong or surrogate
        }
        len = 3;
        codepoint = c & 0x0F;
    } else if ((c & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if (n < 4) return -1;
        if ((c & 0xFC) == 0xF0 && (s[1] & 0xC0) == 0x80) {
            // 4-byte starting with 11110000 is overlong
            return -1;
        }
        if (c > 0xF4) return -1; // > U+10FFFF
        len = 4;
        codepoint = c & 0x07;
    } else {
        // Invalid leading byte (5- or 6-byte sequences or invalid)
        return -1;
    }

    // Check continuation bytes and build codepoint
    for (int i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) {
            return -1; // Invalid continuation byte
        }
        codepoint = (codepoint << 6) | (s[i] & 0x3F);
    }

    // Validate codepoint
    if (len == 2 && codepoint < 0x80) return -1; // overlong
    if (len == 3 && codepoint < 0x800) return -1; // overlong
    if (len == 4 && codepoint < 0x10000) return -1; // overlong

    // Check for surrogates (U+D800..U+DFFF)
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
        return -1;
    }

    // Check upper bound (U+10FFFF)
    if (codepoint > 0x10FFFF) {
        return -1;
    }

    *cp = codepoint;
    return len;
}