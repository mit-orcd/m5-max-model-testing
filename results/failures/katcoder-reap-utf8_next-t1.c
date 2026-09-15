#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    uint32_t codepoint;
    if ((*s & 0x80) == 0) {
        codepoint = *s;
    } else if ((*s & 0x40) != 0) {
        if (n < 2) return -1;
        if ((*s & 0x20) != 0) {
            if ((*s & 0x10) != 0) {
                if (n < 4) return -1;
                if ((*s & 0x08) == 0) return -1;
                if (*s == 0xEF && n >= 4) {
                    const unsigned char *t = s + 3;
                    if ((*t & 0x80) == 0x00 && (*(t+1) & 0x80) == 0x80 && (*(t+2) & 0x80) == 0x80) return -1;
                }
                codepoint = (((uint32_t)(*s & 0x07)) << 18) | (((uint32_t)*((s+1))) << 12) | (((uint32_t)*((s+2)) & 0x3F) << 6) | (*(s+3) & 0x3F);
                if (codepoint > 0x10FFFF) return -1;
                if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
                return 4;
            } else {
                if (n < 3) return -1;
                codepoint = (((uint32_t)(*s & 0x1F)) << 12) | (((uint32_t)*((s+1)) & 0x3F) << 6) | (*(s+2) & 0x3F);
                if (codepoint > 0xFFFF) return -1;
                if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
                return 3;
            }
        } else {
            if (n < 3) return -1;
            codepoint = (((uint32_t)(*s & 0x30)) << 12) | (((uint32_t)*((s+1)) & 0x3F) << 6) | (*(s+2) & 0x3F);
            if (codepoint >= 0x10000) return -1;
            return 3;
        }
    } else if (n < 2) {
        return -1;
    } else {
        codepoint = (((uint32_t)(*s & 0x60)) << 6) | (*(s+1) & 0x3F);
        if (codepoint >= 0x80) return -1;
        return 2;
    }
    *cp = codepoint;
    return 1;
}