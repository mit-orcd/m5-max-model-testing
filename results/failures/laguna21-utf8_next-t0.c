#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t codepoint;
    int num_bytes;

    if ((s[0] & 0x80) == 0) {
        *cp = s[0];
        return 1;
    }

    if ((s[0] & 0xE0) == 0xC0) {
        num_bytes = 2;
        codepoint = s[0] & 0x1F;
    } else if ((s[0] & 0xF0) == 0xE0) {
        num_bytes = 3;
        codepoint = s[0] & 0x0F;
    } else if ((s[0] & 0xF8) == 0xF0) {
        num_bytes = 4;
        codepoint = s[0] & 0x07;
    } else {
        return -1;
    }

    if (n < (size_t)num_bytes) return -1;

    for (int i = 1; i < num_bytes; i++) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        codepoint = (codepoint << 6) | (s[i] & 0x3F);
    }

    if (num_bytes == 1) {
        return -1;
    }

    if (num_bytes == 2 && codepoint < 0x80) return -1;
    if (num_bytes == 3 && codepoint < 0x800) return -1;
    if (num_bytes == 4 && codepoint < 0x10000) return -1;

    if (codepoint > 0x10FFFF) return -1;

    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;

    *cp = codepoint;
    return num_bytes;
}