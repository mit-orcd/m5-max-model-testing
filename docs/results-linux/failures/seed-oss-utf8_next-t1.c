#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (s == NULL && n > 0) return -1;
    if (n == 0) return -1;

    unsigned char lead = s[0];
    uint32_t code_point;

    if (lead < 0x80) {
        *cp = lead;
        return 1;
    } else if (lead < 0xC0) {
        return -1;
    } else if (lead < 0xE0) {
        if (n < 2 || s[1] < 0x80 || s[1] > 0xBF) return -1;
        code_point = (lead & 0x1F) << 6 | (s[1] & 0x3F);
        if (code_point < 0x80) return -1;
        *cp = code_point;
        return 2;
    } else if (lead < 0xF0) {
        if (n < 3 || s[1] < 0x80 || s[1] > 0xBF || s[2] < 0x80 || s[2] > 0xBF) return -1;
        code_point = (lead & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
        if (code_point < 0x800 || (code_point >= 0xD800 && code_point <= 0xDFFF)) return -1;
        *cp = code_point;
        return 3;
    } else if (lead < 0xF8) {
        if (n < 4 || s[1] < 0x80 || s[1] > 0xBF || s[2] < 0x80 || s[2] > 0xBF || s[3] < 0x80 || s[3] > 0xBF) return -1;
        code_point = (lead & 0x07) << 18 | (s[1] & 0x3F) << 12 | (s[2] & 0x3F) << 6 | (s[3] & 0x3F);
        if (code_point < 0x10000 || code_point > 0x10FFFF) return -1;
        *cp = code_point;
        return 4;
    } else {
        return -1;
    }
}