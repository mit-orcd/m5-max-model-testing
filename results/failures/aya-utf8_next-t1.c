#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s[0] <= 0x7f) return -1;
    uint32_t u = s[0];
    size_t i = 1;
    if ((u >> 5) == 0x06) { // 5-byte
        if (i + 5 > n || s[i++] != 0x80 || s[i++] != 0x80 || s[i++] != 0x80 || s[i++] != 0x80 ||
            (s[i++] != 0xf8 && s[i++] != 0xf0))
            return -1;
        *cp = (u << 13) + (s[i++] & 0x3f) + (s[i++] & 0x3f) + (s[i++] & 0x3f) + (s[i++] & 0x3f) + (s[i++] & 0x3f);
    } else if ((u >> 4) == 0x0e) { // 4-byte
        if (i + 4 > n || s[i++] != 0x80 || s[i++] != 0x80 || s[i++] != 0x80 ||
            (s[i++] != 0xf0 && s[i++] != 0xf8))
            return -1;
        *cp = (u << 12) + (s[i++] & 0x3f) + (s[i++] & 0x3f) + (s[i++] & 0x3f) + (s[i++] & 0x3f);
    } else if ((u >> 3) == 0x1e) { // 3-byte
        if (i + 3 > n || s[i++] != 0x80 || s[i++] != 0x80 ||
            (s[i++] != 0xe0 && s[i++] != 0xf0))
            return -1;
        *cp = (u << 11) + (s[i++] & 0x3f) + (s[i++] & 0x3f) + (s[i++] & 0x3f);
    } else if ((u >> 2) == 0x3e) { // 2-byte
        if (i + 2 > n || s[i++] != 0x80 ||
            (s[i++] != 0xc0 && s[i++] != 0xe0))
            return -1;
        *cp = (u << 10) + (s[i++] & 0x3f) + (s[i++] & 0x3f);
    } else { // 1-byte
        *cp = u;
    }
    if (*cp > 0x10ffff || (*cp >= 0xd800 && *cp <= 0xdfff)) return -1;
    return i;
}