#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) {
        return -1;
    }

    uint32_t code_point = 0;
    size_t bytes_consumed = 0;
    int expected_continuation = 0;
    int i;

    unsigned char c = s[0];

    if ((c & 0x80) == 0) {
        // 1-byte ASCII
        code_point = c;
        bytes_consumed = 1;
    } else if ((c & 0xE0) == 0xC0) {
        // 2-byte sequence
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        code_point = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        bytes_consumed = 2;
        // Overlong check: must be >= 0x80
        if (code_point < 0x80) return -1;
    } else if ((c & 0xF0) == 0xE0) {
        // 3-byte sequence
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        code_point = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        bytes_consumed = 3;
        // Overlong check: must be >= 0x800
        if (code_point < 0x800) return -1;
        // Surrogate check: must not be in D800-DFFF
        if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
    } else if ((c & 0xF8) == 0xF0) {
        // 4-byte sequence
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        if ((s[3] & 0xC0) != 0x80) return -1;
        code_point = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        bytes_consumed = 4;
        // Overlong check: must be >= 0x10000
        if (code_point < 0x10000) return -1;
        // Range check: must be <= 0x10FFFF
        if (code_point > 0x10FFFF) return -1;
    } else {
        // Invalid leading byte (continuation or 5/6 byte form or reserved)
        return -1;
    }

    *cp = code_point;
    return (int)bytes_consumed;
}