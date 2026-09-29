#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    const unsigned char b0 = s[0];
    int length;

    if (b0 < 0x80) {
        length = 1;
    } else if (b0 < 0xC0) {
        return -1;
    } else if (b0 < 0xE0) {
        length = 2;
    } else if (b0 < 0xF0) {
        length = 3;
    } else if (b0 < 0xF8) {
        length = 4;
    } else {
        return -1;
    }

    if (n < (size_t)length) return -1;

    for (int i = 1; i < length; i++) {
        if (s[i] < 0x80 || s[i] > 0xBF) return -1;
    }

    uint32_t code_point;
    switch (length) {
        case 1: code_point = b0; break;
        case 2: code_point = (b0 & 0x1F) << 6 | (s[1] & 0x3F); break;
        case 3: code_point = (b0 & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F); break;
        case 4: code_point = (b0 & 0x07) << 18 | (s[1] & 0x3F) << 12 | (s[2] & 0x3F) << 6 | (s[3] & 0x3F); break;
        default: return -1;
    }

    if ((length == 2 && code_point <= 0x7F) || (length == 3 && code_point <= 0x7FF) || (length == 4 && code_point <= 0xFFFF)) {
        return -1;
    }

    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
    if (code_point > 0x10FFFF) return -1;

    *cp = code_point;
    return length;
}