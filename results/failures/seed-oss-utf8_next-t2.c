#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n < 1 || !s || !cp) return -1;
    unsigned char b0 = s[0];
    if (b0 < 0x80) {
        if (n < 1) return -1;
        *cp = b0;
        return 1;
    }
    if (b0 == 0xC0 || b0 == 0xC1) return -1;
    if (b0 >= 0xF5) return -1;
    int len;
    if (b0 < 0xE0) len = 2;
    else if (b0 < 0xF0) len = 3;
    else len = 4;
    if (n < len) return -1;
    for (int i = 1; i < len; i++) {
        if (s[i] < 0x80 || s[i] > 0xBF) return -1;
    }
    uint32_t c;
    if (len == 2) c = (b0 & 0x1F) << 6 | (s[1] & 0x3F);
    else if (len == 3) c = (b0 & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
    else c = (b0 & 0x07) << 18 | (s[1] & 0x3F) << 12 | (s[2] & 0x3F) << 6 | (s[3] & 0x3F);
    if (c < 0x80 && len > 1) return -1;
    if (c >= 0xD800 && c <= 0xDFFF) return -1;
    if (c > 0x10FFFF) return -1;
    *cp = c;
    return len;
}