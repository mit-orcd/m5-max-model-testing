#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t c = s[0];
    int bytes = 0;

    if ((c & 0x80) == 0) {
        // 1-byte sequence: 0xxxxxxx
        bytes = 1;
        if (n < 1) return -1;
        *cp = c;
        return bytes;
    }

    if ((c & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        bytes = 2;
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        *cp = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        // Check overlong (min value for 2 bytes is 0x80)
        if (*cp < 0x80) return -1;
        // Check surrogate and range
        if (*cp >= 0xD800 && *cp <= 0xDFFF) return -1;
        if (*cp > 0x7FF) return -1;
        return bytes;
    }

    if ((c & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        bytes = 3;
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        *cp = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        // Check overlong (min value for 3 bytes is 0x800)
        if (*cp < 0x800) return -1;
        // Check surrogate
        if (*cp >= 0xD800 && *cp <= 0xDFFF) return -1;
        if (*cp > 0xFFFF) return -1;
        return bytes;
    }

    if ((c & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        bytes = 4;
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        if ((s[3] & 0xC0) != 0x80) return -1;
        *cp = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        // Check overlong (min value for 4 bytes is 0x10000)
        if (*cp < 0x10000) return -1;
        // Check max valid code point
        if (*cp > 0x10FFFF) return -1;
        return bytes;
    }

    // Invalid leading byte (10xxxxxx or 11111xxx etc)
    return -1;
}