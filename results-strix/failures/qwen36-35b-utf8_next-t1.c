#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t code_point = 0;
    size_t bytes_needed = 0;

    // Determine expected length and mask based on the leading byte
    if ((s[0] & 0x80) == 0x00) {
        // 1-byte sequence: 0xxxxxxx
        code_point = s[0] & 0x7F;
        bytes_needed = 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        bytes_needed = 2;
        code_point = (s[0] & 0x1F) << 6 | (s[1] & 0x3F);
    } else if ((s[0] & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        bytes_needed = 3;
        code_point = (s[0] & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
    } else if ((s[0] & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        if ((s[3] & 0xC0) != 0x80) return -1;
        bytes_needed = 4;
        code_point = (s[0] & 0x07) << 18 | (s[1] & 0x3F) << 12 | (s[2] & 0x3F) << 6 | (s[3] & 0x3F);
    } else {
        // Invalid leading byte (includes 5/6 byte starters or invalid patterns)
        return -1;
    }

    // Check for overlong encodings
    if (bytes_needed == 2 && code_point < 0x80) return -1;
    if (bytes_needed == 3 && code_point < 0x800) return -1;
    if (bytes_needed == 4 && code_point < 0x10000) return -1;

    // Check for maximum valid code point
    if (code_point > 0x10FFFF) return -1;

    // Check for UTF-16 surrogates
    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;

    *cp = code_point;
    return (int)bytes_needed;
}