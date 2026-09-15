#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    unsigned char c0 = s[0];
    if (c0 < 0x80) {
        *cp = c0;
        return 1;
    }
    if (n < 2) return -1;
    if (c0 < 0xc0) {
        if (c0 < 0xa0) {
            if (n < 3) return -1;
            unsigned char c1 = s[1];
            if (c1 < 0x80 || c1 > 0xbf) return -1;
            unsigned char c2 = s[2];
            if (c2 > 0xbf) return -1;
            *cp = ((uint32_t)(c0 & 0x1f) << 12) | ((uint32_t)(c1 & 0x3f) << 6) | (c2 & 0x3f);
            if (*cp < 0x80) return -1;
            return 3;
        }
        if (n < 4) return -1;
        unsigned char c1 = s[1];
        if (c1 > 0xbf) return -1;
        unsigned char c2 = s[2];
        if (c2 > 0xbf) return -1;
        unsigned char c3 = s[3];
        if (c3 > 0xbf) return -1;
        *cp = ((uint32_t)(c0 & 0x0f) << 18) | ((uint32_t)(c1 & 0x3f) << 12) | ((uint32_t)(c2 & 0x3f) << 6) | (c3 & 0x3f);
        if (*cp < 0x4000) return -1;
        if (*cp >= 0xd800 && *cp <= 0xdfff) return -1;
        if (*cp > 0x10ffff) return -1;
        return 4;
    }
    return -1;
}