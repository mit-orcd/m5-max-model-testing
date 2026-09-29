#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t c0 = s[0];
    uint32_t code_point;
    size_t num_bytes;

    if (c0 < 0x80) {
        num_bytes = 1;
        code_point = c0;
    } else if ((c0 & 0xE0) == 0xC0) {
        num_bytes = 2;
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        code_point = (uint32_t)(c0 & 0x1F) << 6 | (uint32_t)(s[1] & 0x3F);
        if (code_point < 0x80) return -1;
    } else if ((c0 & 0xF0) == 0xE0) {
        num_bytes = 3;
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        code_point = (uint32_t)(c0 & 0x0F) << 12 | (uint32_t)(s[1] & 0x3F) << 6 | (uint32_t)(s[2] & 0x3F);
        if (code_point < 0x800) return -1;
    } else if ((c0 & 0xF8) == 0xF0) {
        num_bytes = 4;
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        if ((s[3] & 0xC0) != 0x80) return -1;
        code_point = (uint32_t)(c0 & 0x07) << 18 | (uint32_t)(s[1] & 0x3F) << 12 | (uint32_t)(s[2] & 0x3F) << 6 | (uint32_t)(s[3] & 0x3F);
        if (code_point < 0x10000) return -1;
        if (code_point > 0x10FFFF) return -1;
    } else {
        return -1;
    }

    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;

    *cp = code_point;
    return (int)num_bytes;
}