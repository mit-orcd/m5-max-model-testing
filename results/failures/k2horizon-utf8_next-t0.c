#include <stddef.h>
#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c = s[0];

    if (c < 0x80) {
        *cp = c;
        return 1;
    }

    size_t len;
    uint32_t value;

    if (c >= 0xC2 && c <= 0xDF) {
        len = 2;
        value = c & 0x1F;
    } else if (c >= 0xE0) {
        if (c <= 0xE0) {
            /* E0: second byte must be 0xA0-0xBF */
            if (n < 3) return -1;
            if (s[1] < 0xA0 || s[1] > 0xBF) return -1;
            value = c & 0x0F;
        } else if (c <= 0xED) {
            len = 3;
            value = c & 0x0F;
        } else if (c <= 0xF4) {
            len = 4;
            value = c & 0x07;
        } else {
            return -1; /* 0xF5-0xFF: invalid */
        }
    } else {
        return -1; /* continuation byte or 0xC0/0xC1 overlong */
    }

    if (n < len) return -1;

    for (size_t i = 1; i < len; i++) {
        unsigned char b = s[i];
        if ((b & 0xC0) != 0x80) return -1;
        value = (value << 6) | (b & 0x3F);
    }

    /* Reject surrogates */
    if (value >= 0xD800 && value <= 0xDFFF) return -1;
    /* Reject above U+10FFFF */
    if (value > 0x10FFFF) return -1;

    *cp = value;
    return (int)len;
}