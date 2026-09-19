#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n < 1 || s[0] > 0x7f)
        return -1;
    uint32_t c = s[0];
    if (c < 0x80) {
        *cp = c;
        return 1;
    }
    if (n < 2 || (s[1] & 0xc0) != 0x80)
        return -1;
    c = (c << 6) | (s[1] & 0x3f);
    if (c < 0x800) {
        *cp = c;
        return 2;
    }
    if (n < 3 || (s[2] & 0xc0) != 0x80)
        return -1;
    c = (c << 6) | (s[2] & 0x3f);
    if (c < 0x10000) {
        *cp = c;
        return 3;
    }
    if (c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff))
        return -1;
    if (n < 4 || (s[3] & 0xc0) != 0x80)
        return -1;
    c = (c << 6) | (s[3] & 0x3f);
    *cp = c;
    return 4;
}