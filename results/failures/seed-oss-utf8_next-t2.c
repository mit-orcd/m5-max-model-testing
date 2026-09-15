#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char b0 = s[0];
    int len;

    if (b0 <= 0x7F) {
        len = 1;
    } else if (b0 >= 0xC0 && b0 <= 0xDF) {
        len = 2;
    } else if (b0 >= 0xE0 && b0 <= 0xEF) {
        len = 3;
    } else if (b0 >= 0xF0 && b0 <= 0xF7) {
        len = 4;
    } else {
        return -1;
    }

    if (n < len) return -1;

    for (int i = 1; i < len; i++) {
        if (s[i] < 0x80 || s[i] > 0xBF) return -1;
    }

    uint32_t code_point;
    switch (len) {
        case 1: code_point = b0; break;
        case 2: code_point = (uint32_t)(b0 & 0x1F) << 6 | (s[1] & 0x3F); break;
        case 3: code_point = (uint32_t)(b0 & 0x0F) << 12 | (uint32_t)(s[1] & 0x3F) << 6 | (s[2] & 0x3F); break;
        case 4: code_point = (uint32_t)(b0 & 0x07) << 18 | (uint32_t)(s[1] & 0x3F) << 12 | (uint32_t)(s[2] & 0x3F) << 6 | (s[3] & 0x3F); break;
        default: return -1;
    }

    if ((len == 2 && code_point <= 0x7F) || (len == 3 && code_point <= 0x7FF) || (len == 4 && code_point <= 0xFFFF))
        return -1;

    if (code_point > 0x10FFFF || (code_point >= 0xD800 && code_point <= 0xDFFF))
        return -1;

    *cp = code_point;
    return len;
}