#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    uint32_t code_point;
    int len;

    if (n == 0) return -1;

    if ((s[0] & 0x80) == 0x00) {
        code_point = s[0];
        len = 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        code_point = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        len = 2;
    } else if ((s[0] & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        code_point = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        len = 3;
    } else if ((s[0] & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        code_point = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        len = 4;
    } else {
        return -1;
    }

    if (code_point <= 0x7F) {
        if (len != 1) return -1;
    } else if (code_point >= 0x80 && code_point <= 0x7FF) {
        if (len != 2) return -1;
    } else if (code_point >= 0x800 && code_point <= 0xFFFF) {
        if (len != 3) return -1;
        if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
    } else if (code_point >= 0x10000 && code_point <= 0x10FFFF) {
        if (len != 4) return -1;
    } else {
        return -1;
    }

    *cp = code_point;
    return len;
}