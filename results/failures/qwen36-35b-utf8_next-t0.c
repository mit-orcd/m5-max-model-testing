#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t code_point = 0;
    size_t bytes_needed = 0;
    size_t i = 0;

    // Determine the number of bytes needed based on the first byte
    if ((s[0] & 0x80) == 0x00) {
        // 1-byte sequence (U+0000 to U+007F)
        bytes_needed = 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        // 2-byte sequence (U+0080 to U+07FF)
        bytes_needed = 2;
    } else if ((s[0] & 0xF0) == 0xE0) {
        // 3-byte sequence (U+0800 to U+FFFF)
        bytes_needed = 3;
    } else if ((s[0] & 0xF8) == 0xF0) {
        // 4-byte sequence (U+10000 to U+10FFFF)
        bytes_needed = 4;
    } else {
        // Invalid leading byte (continuation byte or reserved)
        return -1;
    }

    // Check if we have enough bytes remaining
    if (n < bytes_needed) {
        return -1;
    }

    // Decode the code point
    if (bytes_needed == 1) {
        code_point = s[0];
    } else if (bytes_needed == 2) {
        if ((s[1] & 0xC0) != 0x80) return -1;
        code_point = (s[0] & 0x1F) << 6 | (s[1] & 0x3F);
    } else if (bytes_needed == 3) {
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        code_point = (s[0] & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
    } else if (bytes_needed == 4) {
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        if ((s[3] & 0xC0) != 0x80) return -1;
        code_point = (s[0] & 0x07) << 18 | (s[1] & 0x3F) << 12 | (s[2] & 0x3F) << 6 | (s[3] & 0x3F);
    }

    // Check for overlong encodings
    if (bytes_needed == 2 && code_point < 0x80) return -1;
    if (bytes_needed == 3 && code_point < 0x800) return -1;
    if (bytes_needed == 4 && code_point < 0x10000) return -1;

    // Check for code points above U+10FFFF
    if (code_point > 0x10FFFF) return -1;

    // Check for UTF-16 surrogates (U+D800 to U+DFFF)
    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;

    *cp = code_point;
    return (int)bytes_needed;
}