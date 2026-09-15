#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    uint32_t codepoint;
    int bytes;
    if ((s[0] & 0x80) == 0) {
        codepoint = s[0];
        bytes = 1;
    } else if ((s[0] & 0xe0) == 0xc0) {
        if (n < 2) return -1;
        if ((s[1] & 0xc0) != 0x80) return -1;
        codepoint = ((uint32_t)(s[0] & 0x1f) << 6) | (s[1] & 0x3f);
        bytes = 2;
        if (codepoint < 0x80) return -1;
    } else if ((s[0] & 0xf0) == 0xe0) {
        if (n < 3) return -1;
        if ((s[1] & 0xc0) != 0x80 || (s[2] & 0xc0) != 0x80) return -1;
        codepoint = ((uint32_t)(s[0] & 0x0f) << 12) | ((uint32_t)(s[1] & 0x3f) << 6) | (s[2] & 0x3f);
        bytes = 3;
        if (codepoint < 0x800) return -1;
    } else if ((s[0] & 0xf8) == 0xf0) {
        if (n < 4) return -1;
        if ((s[1] & 0xc0) != 0x80 || (s[2] & 0xc0) != 0x80 || (s[3] & 0xc0) != 0x80) return -1;
        codepoint = ((uint32_t)(s[0] & 0x07) << 18) | ((uint32_t)(s[1] & 0x3f) << 12) | ((uint32_t)(s[2] & 0x3f) << 6) | (s[3] & 0x3f);
        bytes = 4;
        if (codepoint < 0x10000) return -1;
        if (codepoint > 0x10FFFF) return -1;
    } else {
        return -1;
    }
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
    *cp = codepoint;
    return bytes;
}