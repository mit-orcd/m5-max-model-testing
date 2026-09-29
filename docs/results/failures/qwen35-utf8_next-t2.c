#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) {
        return -1;
    }

    uint32_t ucs = 0;
    int bytes_read = 1;
    unsigned char first = s[0];

    // Determine expected length and initial mask
    if ((first & 0x80) == 0) {
        // 1 byte: 0xxxxxxx
        ucs = first;
    } else if ((first & 0xE0) == 0xC0) {
        // 2 bytes: 110xxxxx
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        ucs = (first & 0x1F) << 6 | (s[1] & 0x3F);
        // Check overlong encoding (must be >= 0x80)
        if (ucs < 0x80) return -1;
        bytes_read = 2;
    } else if ((first & 0xF0) == 0xE0) {
        // 3 bytes: 1110xxxx
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        ucs = (first & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
        // Check overlong encoding (must be >= 0x800)
        if (ucs < 0x800) return -1;
        // Check surrogate range (U+D800-U+DFFF)
        if (ucs >= 0xD800 && ucs <= 0xDFFF) return -1;
        // Max valid is U+10FFFF
        if (ucs > 0x10FFFF) return -1;
        bytes_read = 3;
    } else if ((first & 0xF8) == 0xF0) {
        // 4 bytes: 11110xxx
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        if ((s[3] & 0xC0) != 0x80) return -1;
        ucs = (first & 0x07) << 18 | (s[1] & 0x3F) << 12 | (s[2] & 0x3F) << 6 | (s[3] & 0x3F);
        // Check overlong encoding (must be >= 0x10000)
        if (ucs < 0x10000) return -1;
        // Check max valid (U+10FFFF)
        if (ucs > 0x10FFFF) return -1;
        bytes_read = 4;
    } else {
        // Invalid leading byte (continuation byte, or 5/6 byte forms, or > 0xF7)
        return -1;
    }

    *cp = ucs;
    return bytes_read;
}