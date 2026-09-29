#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    
    unsigned char b0 = s[0];
    
    if (b0 < 0x80) {
        *cp = b0;
        return 1;
    }
    
    if (b0 < 0xC0) {
        return -1;
    }
    
    if (b0 >= 0xFE) {
        return -1;
    }
    
    if (b0 >= 0xF8) {
        return -1;
    }
    
    if (b0 >= 0xF0) {
        if (n < 4) return -1;
        unsigned char b1 = s[1], b2 = s[2], b3 = s[3];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) {
            return -1;
        }
        uint32_t codepoint = ((uint32_t)(b0 & 0x07) << 18) |
                            ((uint32_t)(b1 & 0x3F) << 12) |
                            ((uint32_t)(b2 & 0x3F) << 6)  |
                            ((uint32_t)(b3 & 0x3F));
        if (codepoint > 0x10FFFF) return -1;
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
        if (codepoint < 0x10000) return -1;
        *cp = codepoint;
        return 4;
    }
    
    if (b0 >= 0xE0) {
        if (n < 3) return -1;
        unsigned char b1 = s[1], b2 = s[2];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) {
            return -1;
        }
        uint32_t codepoint = ((uint32_t)(b0 & 0x0F) << 12) |
                            ((uint32_t)(b1 & 0x3F) << 6)  |
                            ((uint32_t)(b2 & 0x3F));
        if (codepoint < 0x800) return -1;
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
        *cp = codepoint;
        return 3;
    }
    
    if (n < 2) return -1;
    {
        unsigned char b1 = s[1];
        if ((b1 & 0xC0) != 0x80) return -1;
        uint32_t codepoint = ((uint32_t)(b0 & 0x1F) << 6) | ((uint32_t)(b1 & 0x3F));
        if (b0 >= 0xC2) {
            if (codepoint < 0x80) return -1;
            if (codepoint < 0x800) {
                if (b0 == 0xC2 && codepoint < 0x80) return -1;
            }
            *cp = codepoint;
            return 2;
        }
        return -1;
    }
}