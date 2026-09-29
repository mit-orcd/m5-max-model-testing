#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    uint32_t codepoint;
    int bytes_needed;
    unsigned char first = s[0];

    if (first < 0x80) {
        *cp = first;
        return 1;
    }

    if (first < 0xC0) return -1;
    if (first < 0xE0) { bytes_needed = 2; codepoint = first & 0x1F; }
    else if (first < 0xF0) { bytes_needed = 3; codepoint = first & 0x0F; }
    else if (first < 0xF8) { bytes_needed = 4; codepoint = first & 0x07; }
    else return -1;

    if (n < (size_t)bytes_needed) return -1;

    for (int i = 1; i < bytes_needed; i++) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        codepoint = (codepoint << 6) | (s[i] & 0x3F);
    }

    if (codepoint > 0x10FFFF) return -1;
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;

    switch (bytes_needed) {
        case 2: if (codepoint < 0x80) return -1; break;
        case 3: if (codepoint < 0x800) return -1; break;
        case 4: if (codepoint < 0x10000) return -1; break;
    }

    *cp = codepoint;
    return bytes_needed;
}