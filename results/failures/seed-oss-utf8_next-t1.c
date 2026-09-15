#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    const unsigned char b0 = *s;
    int len;

    if (b0 < 0x80) len = 1;
    else if (b0 < 0xC0) return -1;
    else if (b0 < 0xE0) len = 2;
    else if (b0 < 0xF0) len = 3;
    else if (b0 < 0xF8) len = 4;
    else return -1;

    if (n < len) return -1;
    for (int i = 1; i < len; i++) if (s[i] < 0x80 || s[i] > 0xBF) return -1;

    uint32_t cp_val;
    switch (len) {
        case 1: cp_val = b0; break;
        case 2: cp_val = ((b0 & 0x1F) << 6) | (s[1] & 0x3F); break;
        case 3: cp_val = ((b0 & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); break;
        case 4: cp_val = ((b0 & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F); break;
        default: return -1;
    }

    if ((len == 2 && cp_val < 0x80) || (len == 3 && cp_val < 0x800) || (len == 4 && cp_val < 0x10000)) return -1;
    if (cp_val >= 0xD800 && cp_val <= 0xDFFF) return -1;
    if (cp_val > 0x10FFFF) return -1;

    *cp = cp_val;
    return len;
}