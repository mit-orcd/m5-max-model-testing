#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (!s || n == 0) {
        return -1;
    }

    unsigned char b0 = s[0];

    /* 1-byte sequence (ASCII) */
    if (b0 < 0x80) {
        *cp = b0;
        return 1;
    }

    /* 2-byte sequence */
    if ((b0 & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        unsigned char b1 = s[1];
        if ((b1 & 0xC0) != 0x80) return -1;

        uint32_t codepoint = ((b0 & 0x1F) << 6) | (b1 & 0x3F);
        
        /* Overlong check */
        if (codepoint < 0x80) return -1;
        
        *cp = codepoint;
        return 2;
    }

    /* 3-byte sequence */
    if ((b0 & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        unsigned char b1 = s[1];
        unsigned char b2 = s[2];
        
        if ((b1 & 0xC0) != 0x80) return -1;
        if ((b2 & 0xC0) != 0x80) return -1;

        uint32_t codepoint = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
        
        /* Overlong check */
        if (codepoint < 0x800) return -1;
        
        /* Surrogate check */
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
        
        /* Max code point check */
        if (codepoint > 0x10FFFF) return -1;

        *cp = codepoint;
        return 3;
    }

    /* 4-byte sequence */
    if ((b0 & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        unsigned char b1 = s[1];
        unsigned char b2 = s[2];
        unsigned char b3 = s[3];
        
        if ((b1 & 0xC0) != 0x80) return -1;
        if ((b2 & 0xC0) != 0x80) return -1;
        if ((b3 & 0xC0) != 0x80) return -1;

        uint32_t codepoint = ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        
        /* Overlong check */
        if (codepoint < 0x10000) return -1;
        
        /* Max code point check */
        if (codepoint > 0x10FFFF) return -1;

        *cp = codepoint;
        return 4;
    }

    /* Invalid leading byte */
    return -1;
}