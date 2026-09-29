#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t lead = s[0];

    if (lead < 0x80) {
        if (n < 1) return -1;
        *cp = lead;
        return 1;
    }

    if ((lead & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        uint32_t b1 = s[0] & 0x1F;
        uint32_t b2 = s[1];
        if ((b2 & 0xC0) != 0x80) return -1;
        uint32_t cp_val = (b1 << 6) | (b2 & 0x3F);
        if (cp_val < 0x80) return -1;
        *cp = cp_val;
        return 2;
    }

    if ((lead & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        uint32_t b1 = s[0] & 0x0F;
        uint32_t b2 = s[1];
        uint32_t b3 = s[2];
        if ((b2 & 0xC0) != 0x80) return -1;
        if ((b3 & 0xC0) != 0x80) return -1;
        uint32_t cp_val = (b1 << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        if (cp_val < 0x800) return -1;
        if (cp_val >= 0xD800 && cp_val <= 0xDFFF) return -1;
        *cp = cp_val;
        return 3;
    }

    if ((lead & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        uint32_t b1 = s[0] & 0x07;
        uint32_t b2 = s[1];
        uint32_t b3 = s[2];
        uint32_t b4 = s[3];
        if ((b2 & 0xC0) != 0x80) return -1;
        if ((b3 & 0xC0) != 0x80) return -1;
        if ((b4 & 0xC0) != 0x80) return -1;
        uint32_t cp_val = (b1 << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6) | (b4 & 0x3F);
        if (cp_val < 0x10000) return -1;
        if (cp_val > 0x10FFFF) return -1;
        *cp = cp_val;
        return 4;
    }

    return -1;
}