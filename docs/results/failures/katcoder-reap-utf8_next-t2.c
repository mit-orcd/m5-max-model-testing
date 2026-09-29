#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    unsigned char c0 = s[0];
    if (c0 < 0x80) {
        *cp = (uint32_t)c0;
        return 1;
    }
    if (n < 2) return -1;
    if (c0 < 0xc0) {
        return -1;
    }
    if (c0 < 0xe0) {
        if (n < 3) return -1;
        unsigned char c1 = s[1];
        if ((c1 & 0x80) == 0) {
            *cp = ((uint32_t)(c0 & 0x1f) << 6) | (uint32_t)(c1 & 0x3f);
            return 2;
        }
        return -1;
    }
    if (c0 < 0xf0) {
        if (n < 3) return -1;
        unsigned char c1 = s[1], c2 = s[2];
        if ((c2 & 0x80) == 0) {
            *cp = ((uint32_t)(c0 & 0x0f) << 12) | ((uint32_t)(c1 & 0x3f) << 6) | (uint32_t)(c2 & 0x3f);
            if (*cp >= 0xd800 && *cp <= 0xdfff) return -1;
            return 3;
        }
        return -1;
    }
    if (c0 < 0xf8) {
        if (n < 4) return -1;
        unsigned char c1 = s[1], c2 = s[2], c3 = s[3];
        if ((c3 & 0x80) == 0) {
            *cp = ((uint32_t)(c0 & 0x0f) << 18) | ((uint32_t)(c1 & 0x3f) << 12) | ((uint32_t)(c2 & 0x3f) << 6) | (uint32_t)(c3 & 0x3f);
            if (*cp >= 0xd800 && *cp <= 0xdfff) return -1;
            if (*cp > 0x10ffff) return -1;
            return 4;
        }
        return -1;
    }
    if (c0 == 0xf8) {
        if (n < 5) return -1;
        if ((s[1] & 0x80) == 0 && (s[2] & 0x80) == 0 && (s[3] & 0x80) == 0 && (s[4] & 0x80) == 0) return -1;
        return -1;
    }
    if (c0 == 0xf9) {
        if (n < 5) return -1;
        if ((s[1] & 0x80) == 0 && (s[2] & 0x80) == 0 && (s[3] & 0x80) == 0 && (s[4] & 0x80) == 0) return -1;
        return -1;
    }
    if (c0 == 0xfa) {
        if (n < 5) return -1;
        if ((s[1] & 0x80) == 0 && (s[2] & 0x80) == 0 && (s[3] & 0x80) == 0 && (s[4] & 0x80) == 0) return -1;
        return -1;
    }
    if (c0 == 0xfb) {
        if (n < 5) return -1;
        if ((s[1] & 0x80) == 0 && (s[2] & 0x80) == 0 && (s[3] & 0x80) == 0 && (s[4] & 0x80) == 0) return -1;
        return -1;
    }
    if (c0 == 0xfc) {
        if (n < 5) return -1;
        unsigned char c1 = s[1], c2 = s[2], c3 = s[3], c4 = s[4];
        if ((c4 & 0x80) == 0) {
            *cp = ((uint32_t)(c0 & 0x03) << 24) | ((uint32_t)(c1 & 0x3f) << 18) | ((uint32_t)(c2 & 0x3f) << 12) | ((uint32_t)(c3 & 0x3f) << 6) | (uint32_t)(c4 & 0x3f);
            if (*cp > 0x10ffff) return -1;
            return 5;
        }
        return -1;
    }
    if (c0 == 0xfd) {
        if (n < 5) return -1;
        if ((s[1] & 0x80) == 0 && (s[2] & 0x80) == 0 && (s[3] & 0x80) == 0 && (s[4] & 0x80) == 0) return -1;
        return -1;
    }
    if (c0 == 0xfe) {
        if (n < 6) return -1;
        if ((s[1] & 0x80) == 0 && (s[2] & 0x80) == 0 && (s[3] & 0x80) == 0 && (s[4] & 0x80) == 0 && (s[5] & 0x80) == 0) return -1;
        return -1;
    }
    if (c0 == 0xff) {
        if (n < 6) return -1;
        if ((s[1] & 0x80) == 0 && (s[2] & 0x80) == 0 && (s[3] & 0x80) == 0 && (s[4] & 0x80) == 0 && (s[5] & 0x80) == 0) return -1;
        return -1;
    }
    return -1;
}