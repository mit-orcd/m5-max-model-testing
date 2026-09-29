#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char lead = s[0];
    int len;

    if (lead <= 0x7F) len = 1;
    else if (lead <= 0xDF) len = 2;
    else if (lead <= 0xEF) len = 3;
    else if (lead <= 0xF7) len = 4;
    else return -1;

    if (n < len) return -1;

    for (int i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) return -1;
    }

    uint32_t code_point;
    switch (len) {
        case 1: code_point = lead; break;
        case 2: code_point = ((lead & 0x1F) << 6) | (s[1] & 0x3F); break;
        case 3: code_point = ((lead & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); break;
        case 4: code_point = ((lead & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F); break;
        default: return -1;
    }

    if ((len == 2 && code_point < 0x80) || (len == 3 && code_point < 0x800) || (len == 4 && code_point < 0x10000))
        return -1;

    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
    if (code_point > 0x10FFFF) return -1;

    *cp = code_point;
    return len;
}