#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t lead = s[0];

    if (lead < 0x80) {
        *cp = lead;
        return 1;
    }

    if (lead >= 0xC0 && lead <= 0xDF) {
        if (n < 2) return -1;
        uint32_t b1 = s[1];
        if ((b1 & 0xC0) != 0x80) return -1;
        uint32_t cp_val = (lead & 0x1F) << 6 | (b1 & 0x3F);
        if (cp_val < 0x80) return -1;
        *cp = cp_val;
        return 2;
    }

    if (lead >= 0xE0 && lead <= 0xEF) {
        if (n < 3) return -1;
        uint32_t b1 = s[1];
        uint32_t b2 = s[2];
        if ((b1 & 0xC0) != 0x80) return -1;
        if ((b2 & 0xC0) != 0x80) return -1;
        uint32_t cp_val = (lead & 0x0F) << 12 | (b1 & 0x3F) << 6 | (b2 & 0x3F);
        if (cp_val < 0x800) return -1;
        if (cp_val >= 0xD800 && cp_val <= 0xDFFF) return -1;
        *cp = cp_val;
        return 3;
    }

    if (lead >= 0xF0 && lead <= 0xF7) {
        if (n < 4) return -1;
        uint32_t b1 = s[1];
        uint32_t b2 = s[2];
        uint32_t b3 = s[3];
        if ((b1 & 0xC0) != 0x80) return -1;
        if ((b2 & 0xC0) != 0x80) return -1;
        if ((b3 & 0xC0) != 0x80) return -1;
        uint32_t cp_val = (lead & 0x07) << 18 | (b1 & 0x3F) << 12 | (b1 & 0x3F) << 6 | (b3 & 0x3F);
        if (cp_val < 0x10000) return -1;
        if (cp_val > 0x10FFFF) return -1;
        *cp = cp_val;
        return 4;
    }

    return -1;
}