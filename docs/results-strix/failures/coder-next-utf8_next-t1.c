#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s == NULL || cp == NULL) {
        return -1;
    }

    unsigned char c = s[0];
    size_t len;

    if ((c & 0x80) == 0) {
        *cp = c;
        return 1;
    }

    if ((c & 0xE0) == 0xC0) {
        len = 2;
        if (n < 2) return -1;
        if ((c & 0x1F) == 0) return -1; // overlong 2-byte
        if ((c & 0x1F) == 1 && (s[1] & 0x80) == 0x80 && (s[1] & 0x40) == 0) {
            // 0xC0 or 0xC1 lead bytes are overlong
            return -1;
        }
    } else if ((c & 0xF0) == 0xE0) {
        len = 3;
        if (n < 3) return -1;
    } else if ((c & 0xF8) == 0xF0) {
        len = 4;
        if (n < 4) return -1;
    } else {
        return -1;
    }

    // Check continuation bytes
    for (size_t i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) {
            return -1;
        }
    }

    // Decode code point
    uint32_t codepoint;
    switch (len) {
        case 2:
            codepoint = (c & 0x1F) << 6 | (s[1] & 0x3F);
            break;
        case 3:
            codepoint = (c & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
            break;
        case 4:
            codepoint = (c & 0x07) << 18 | (s[1] & 0x3F) << 12 | (s[2] & 0x3F) << 6 | (s[3] & 0x3F);
            break;
        default:
            return -1;
    }

    // Check for overlong encoding
    if ((len == 2 && codepoint < 0x80) ||
        (len == 3 && codepoint < 0x800) ||
        (len == 4 && codepoint < 0x10000)) {
        return -1;
    }

    // Check for surrogate pairs (U+D800..U+DFFF)
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
        return -1;
    }

    // Check for code points above U+10FFFF
    if (codepoint > 0x10FFFF) {
        return -1;
    }

    *cp = codepoint;
    return (int)len;
}