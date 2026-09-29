#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp)
{
    if (n == 0 || s == NULL || cp == NULL)
        return -1;

    unsigned char c = s[0];
    size_t len;
    uint32_t codepoint;

    if (c < 0x80) {
        // 1-byte sequence: 0xxxxxxx
        *cp = c;
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        if (n < 2 || (s[1] & 0xC0) != 0x80)
            return -1;
        if ((c & 0xFE) == 0xC0) // overlong check (0xC0, 0xC1)
            return -1;
        len = 2;
        codepoint = c & 0x1F;
    } else if ((c & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        if (n < 3 || (s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80)
            return -1;
        len = 3;
        codepoint = c & 0x0F;
    } else if ((c & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if (n < 4 || (s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80)
            return -1;
        if ((c & 0xF8) != 0xF0) // 5/6-byte sequences (obsolete)
            return -1;
        if (c > 0xF4) // > U+10FFFF
            return -1;
        len = 4;
        codepoint = c & 0x07;
    } else {
        // Invalid leading byte (e.g., continuation byte or 5/6-byte header)
        return -1;
    }

    // Accumulate remaining bytes
    for (size_t i = 1; i < len; i++) {
        codepoint = (codepoint << 6) | (s[i] & 0x3F);
    }

    // Additional validation
    if (len == 2 && codepoint < 0x80) // overlong
        return -1;
    if (len == 3 && codepoint < 0x800) // overlong
        return -1;
    if (len == 4 && codepoint < 0x10000) // overlong
        return -1;
    if (codepoint > 0x10FFFF) // beyond Unicode range
        return -1;
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) // surrogate pair
        return -1;

    *cp = codepoint;
    return (int)len;
}