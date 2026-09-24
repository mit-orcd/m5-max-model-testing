#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char b0 = s[0];
    uint32_t codepoint;
    size_t len;

    if ((b0 & 0x80) == 0x00) {
        // 1-byte sequence: 0xxxxxxx
        codepoint = b0;
        len = 1;
    } else if ((b0 & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx
        codepoint = b0 & 0x1F;
        len = 2;
    } else if ((b0 & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx
        codepoint = b0 & 0x0F;
        len = 3;
    } else if ((b0 & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx
        codepoint = b0 & 0x07;
        len = 4;
    } else {
        // Invalid leading byte (continuation or invalid)
        return -1;
    }

    if (n < len) {
        // Truncated sequence
        return -1;
    }

    // Process continuation bytes
    for (size_t i = 1; i < len; ++i) {
        unsigned char b = s[i];
        if ((b & 0xC0) != 0x80) {
            // Bad continuation byte
            return -1;
        }
        codepoint = (codepoint << 6) | (b & 0x3F);
    }

    // Validate based on sequence length
    switch (len) {
        case 2:
            // Must be >= U+0080, else overlong
            if (codepoint < 0x80) return -1;
            break;
        case 3:
            // Must be >= U+0800, else overlong
            if (codepoint < 0x800) return -1;
            // Surrogates U+D800..U+DFFF are invalid
            if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
            break;
        case 4:
            // Must be >= U+10000, else overlong
            if (codepoint < 0x10000) return -1;
            // Must be <= U+10FFFF
            if (codepoint > 0x10FFFF) return -1;
            break;
    }

    *cp = codepoint;
    return (int)len;
}