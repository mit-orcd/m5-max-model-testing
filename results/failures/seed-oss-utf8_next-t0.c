#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n < 1) return -1;
    unsigned char b0 = s[0];
    if (b0 < 0x80) {
        if (n < 1) return -1;
        *cp = b0;
        return 1;
    } else if (b0 == 0xC0 || b0 == 0xC1) {
        return -1;
    } else if (b0 >= 0xE0 && b0 < 0xF0) {
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        uint32_t c = ((b0 & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        if (c < 0x800) return -1;
        *cp = c;
        return 3;
    } else if (b0 >= 0xF0 && b0 < 0xF8) {
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        uint32_t c = ((b0 & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        if (c > 0x10FFFF || c < 0x10000) return -1;
        *cp = c;
        return 4;
    } else {
        return -1;
    }
}