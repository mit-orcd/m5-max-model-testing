#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t codepoint;
    size_t len;
    uint32_t min_val;

    if ((s[0] & 0x80) == 0x00) {
        *cp = s[0];
        return 1;
    }

    if ((s[0] & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        codepoint = ((uint32_t)(s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        len = 2;
        min_val = 0x80;
    } else if ((s[0] & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        codepoint = ((uint32_t)(s[0] & 0x0F) << 12) |
                    ((uint32_t)(s[1] & 0x3F) << 6) |
                    (s[2] & 0x3F);
        len = 3;
        min_val = 0x800;
    } else if ((s[0] & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        if ((s[3] & 0xC0) != 0x80) return -1;
        codepoint = ((uint32_t)(s[0] & 0x07) << 18) |
                    ((uint32_t)(s[1] & 0x3F) << 12) |
                    ((uint32_t)(s[2] & 0x3F) << 6) |
                    (s[3] & 0x3F);
        len = 4;
        min_val = 0x10000;
    } else {
        return -1;
    }

    if (codepoint < min_val) return -1;

    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;

    if (codepoint > 0x10FFFF) return -1;

    *cp = codepoint;
    return (int)len;
}