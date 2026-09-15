#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char b0 = s[0];
    if (b0 < 0x80) {
        *cp = b0;
        return 1;
    }
    if (b0 < 0xC0) return -1;

    if (b0 <= 0xDF) { /* 2-byte */
        if (n < 2) return -1;
        unsigned char b1 = s[1];
        if ((b1 & 0xC0) != 0x80) return -1;
        uint32_t cp_val = ((b0 & 0x1F) << 6) | (b1 & 0x3F);
        if (cp_val < 0x80) return -1;
        *cp = cp_val;
        return 2;
    }

    if (b0 <= 0xEF) { /* 3-byte */
        if (n < 3) return -1;
        unsigned char b1 = s[1], b2 = s[2];
        if (((b1 | b2) & 0xC0) != 0x80) return -1;
        uint32_t cp_val = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
        if (cp_val < 0x800 || (cp_val >= 0xD800 && cp_val <= 0xDFFF)) return -1;
        *cp = cp_val;
        return 3;
    }

    if (b0 <= 0xF4) { /* 4-byte */
        if (n < 4) return -1;
        unsigned char b1 = s[1], b2 = s[2], b3 = s[3];
        if (((b1 | b2 | b3) & 0xC0) != 0x80) return -1;
        uint32_t cp_val = ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        if (cp_val < 0x10000 || cp_val > 0x10FFFF) return -1;
        *cp = cp_val;
        return 4;
    }

    return -1;
}