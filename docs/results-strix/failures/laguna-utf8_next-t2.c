#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t codepoint;
    uint8_t first = s[0];

    if (first < 0x80) {
        *cp = first;
        return 1;
    }

    if ((first & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        uint8_t second = s[1];
        if ((second & 0xC0) != 0x80) return -1;

        codepoint = (first & 0x1F) << 6 | (second & 0x3F);

        if (codepoint < 0x80) return -1;

        *cp = codepoint;
        return 2;
    }

    if ((first & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        uint8_t second = s[1];
        uint8_t third = s[2];
        if ((second & 0xC0) != 0x80) return -1;
        if ((third & 0xC0) != 0x80) return -1;

        codepoint = (first & 0x0F) << 12 | (second & 0x3F) << 6 | (third & 0x3F);

        if (codepoint < 0x800) return -1;
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;

        *cp = codepoint;
        return 3;
    }

    if ((first & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        uint8_t second = s[1];
        uint8_t third = s[2];
        uint8_t fourth = s[3];
        if ((second & 0xC0) != 0x80) return -1;
        if ((third & 0xC0) != 0x80) return -1;
        if ((fourth & 0xC0) != 0x80) return -1;

        codepoint = (first & 0x07) << 18 | (second & 0x3F) << 12 | (third & 0x3F) << 6 | (fourth & 0x3F);

        if (codepoint < 0x10000) return -1;
        if (codepoint > 0x10FFFF) return -1;

        *cp = codepoint;
        return 4;
    }

    return -1;
}