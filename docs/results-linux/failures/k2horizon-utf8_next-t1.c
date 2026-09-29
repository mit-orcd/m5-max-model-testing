#include <stddef.h>
#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c0 = s[0];

    if (c0 < 0x80) {
        *cp = c0;
        return 1;
    }

    unsigned char len;
    uint32_t cp_val;

    if (c0 >= 0xC2 && c0 <= 0xDF) {
        len = 2;
        cp_val = c0 & 0x1F;
    } else if (c0 >= 0xE0) {
        if (c0 <= 0xE0) {
            len = 3;
            cp_val = c0 & 0x0F;
        } else if (c0 <= 0xEF) {
            len = 3;
            cp_val = c0 & 0x0F;
        } else {
            return -1;
        }
    } else {
        return -1;
    }

    if (n < len) return -1;

    if (len == 2) {
        unsigned char c1 = s[1];
        if ((c1 & 0xC0) != 0x80) return -1;
        cp_val = (cp_val << 6) | (c1 & 0x3F);
        if (cp_val < 0x80) return -1; /* overlong */
    } else {
        /* 3-byte sequence */
        if (c0 >= 0xE0 && c0 <= 0xEF) {
            /* restrict range based on first byte */
            if (c0 == 0xE0) {
                /* cp_val must be >= 0xA0 in first continuation */
                unsigned char c1 = s[1];
                if ((c1 & 0xC0) != 0x80) return -1;
                cp_val = (cp_val << 6) | (c1 & 0x3F);
                if (cp_val < 0xA0) return -1; /* overlong */
            } else if (c0 == 0xED) {
                unsigned char c1 = s[1];
                if ((c1 & 0xC0) != 0x80) return -1;
                cp_val = (cp_val << 6) | (c1 & 0x3F);
                if (cp_val >= 0x900) return -1; /* surrogates U+D800-DFFF */
            } else {
                unsigned char c1 = s[1];
                if ((c1 & 0xC0) != 0x80) return -1;
                cp_val = (cp_val << 6) | (c1 & 0x3F);
            }
            if (len == 3) {
                unsigned char c2 = s[2];
                if ((c2 & 0xC0) != 0x80) return -1;
                cp_val = (cp_val << 6) | (c2 & 0x3F);
            }
        }
    }

    if (cp_val > 0x10FFFF) return -1;

    *cp = cp_val;
    return (int)len;
}