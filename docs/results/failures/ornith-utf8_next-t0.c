#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) {
        return -1;
    }

    uint32_t cpv = s[0];

    if (cpv < 0x80) {
        *cp = cpv;
        return 1;
    }

    if (cpv < 0xC0) {
        /* Continuation byte in leading position */
        return -1;
    }

    if (cpv < 0xE0) {
        if (n < 2) {
            return -1;
        }
        if ((s[1] & 0xC0) != 0x80) {
            return -1;
        }
        cpv = ((cpv & 0x1F) << 6) | (s[1] & 0x3F);
        /* Overlong check: 2-byte form must encode >= 0x80 */
        if (cpv < 0x80) {
            return -1;
        }
        *cp = cpv;
        return 2;
    }

    if (cpv < 0xF0) {
        if (n < 3) {
            return -1;
        }
        if ((s[1] & 0xC0) != 0x80) {
            return -1;
        }
        if ((s[2] & 0xC0) != 0x80) {
            return -1;
        }
        cpv = ((cpv & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        /* Overlong check: 3-byte form must encode >= 0x800 */
        if (cpv < 0x800) {
            return -1;
        }
        /* Surrogate range */
        if (cpv >= 0xD800 && cpv <= 0xDFFF) {
            return -1;
        }
        *cp = cpv;
        return 3;
    }

    if (cpv < 0xF8) {
        /* 4-byte form (and obsolete 5/6-byte forms rejected here) */
        if (n < 4) {
            return -1;
        }
        if ((s[1] & 0xC0) != 0x80) {
            return -1;
        }
        if ((s[2] & 0xC0) != 0x80) {
            return -1;
        }
        if ((s[3] & 0xC0) != 0x80) {
            return -1;
        }
        cpv = ((cpv & 0x07) << 18) | ((s[1] & 0x3F) << 12) |
              ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        /* Overlong check: 4-byte form must encode >= 0x10000 */
        if (cpv < 0x10000) {
            return -1;
        }
        /* Above maximum code point */
        if (cpv > 0x10FFFF) {
            return -1;
        }
        *cp = cpv;
        return 4;
    }

    /* 0xF8 and above: obsolete 5- and 6-byte forms */
    return -1;
}