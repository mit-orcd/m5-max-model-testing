#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint8_t first = s[0];

    if (first < 0x80) {
        *cp = first;
        return 1;
    }

    uint32_t codepoint;
    int expected_len;

    if ((first & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        expected_len = 2;
        codepoint = first & 0x1F;
    } else if ((first & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        expected_len = 3;
        codepoint = first & 0x0F;
    } else if ((first & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        expected_len = 4;
        codepoint = first & 0x07;
    } else {
        return -1;
    }

    if (expected_len == 2 && codepoint < 0x02) return -1;
    if (expected_len == 3 && codepoint < 0x10) return -1;
    if (expected_len == 4 && codepoint < 0x08) return -1;

    for (int i = 1; i < expected_len; i++) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        codepoint = (codepoint << 6) | (s[i] & 0x3F);
    }

    if (expected_len == 2 && codepoint < 0x80) return -1;
    if (expected_len == 3 && (codepoint < 0x80 || codepoint > 0xFFFF)) return -1;
    if (expected_len == 4 && (codepoint < 0x10000 || codepoint > 0x10FFFF)) return -1;

    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;

    *cp = codepoint;
    return expected_len;
}