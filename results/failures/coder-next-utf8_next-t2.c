#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || cp == NULL) {
        return -1;
    }

    unsigned char c = s[0];
    int bytes;
    uint32_t codepoint;

    if ((c & 0x80) == 0) {
        // 1-byte sequence (ASCII)
        *cp = c;
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        // 2-byte sequence
        if (n < 2) return -1;
        bytes = 2;
        codepoint = c & 0x1F;
        // Check for overlong: must be >= 0x80
        if (codepoint < 0x02) return -1;
    } else if ((c & 0xF0) == 0xE0) {
        // 3-byte sequence
        if (n < 3) return -1;
        bytes = 3;
        codepoint = c & 0x0F;
        // Check for overlong or surrogate: must be >= 0x01 and not in surrogate range
        if (codepoint == 0) return -1; // overlong
    } else if ((c & 0xF8) == 0xF0) {
        // 4-byte sequence
        if (n < 4) return -1;
        bytes = 4;
        codepoint = c & 0x07;
        // Check for overlong: must be >= 0x01
        if (codepoint == 0) return -1;
    } else {
        // Invalid leading byte (5/6-byte or bad format)
        return -1;
    }

    // Process continuation bytes
    for (int i = 1; i < bytes; ++i) {
        if (i >= n) return -1;
        unsigned char cont = s[i];
        if ((cont & 0xC0) != 0x80) return -1; // Not a continuation byte
        codepoint = (codepoint << 6) | (cont & 0x3F);
    }

    // Check for overlong encoding
    if (bytes == 2 && codepoint < 0x80) return -1;
    if (bytes == 3 && codepoint < 0x800) return -1;
    if (bytes == 4 && codepoint < 0x10000) return -1;

    // Check for Unicode surrogates (U+D800..U+DFFF)
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;

    // Check for out of range (> U+10FFFF)
    if (codepoint > 0x10FFFF) return -1;

    *cp = codepoint;
    return bytes;
}