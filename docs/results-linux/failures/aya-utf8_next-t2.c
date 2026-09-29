#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s[0] < 0x20 || (s[0] >= 0xe0 && s[0] <= 0xf4))
        return -1;
    if (s[0] < 0x80) {
        *cp = s[0];
        return 1;
    }
    if (s[0] < 0xc2)
        return -1;
    if (s[0] < 0xe0) {
        if (n < 2 || s[1] < 0x80 || s[1] >= 0xf8)
            return -1;
        *cp = (s[0] << 6) + s[1] - 0x80;
        return 2;
    }
    if (s[0] < 0xf0) {
        if (n < 3 || s[1] < 0x80 || s[1] >= 0xf8 || s[2] < 0x80 || s[2] >= 0xf8)
            return -1;
        *cp = (s[0] << 12) + (s[1] << 6) + s[2] - 0x80;
        if (*cp >= 0x10000 && s[3] != 0xf8)
            return -1;
        if (*cp >= 0x10000) {
            *cp = (s[0] << 16) + (s[1] << 10) + (s[2] << 6) + s[3] - 0x80;
            return 4;
        }
        return 3;
    }
    return -1;
}