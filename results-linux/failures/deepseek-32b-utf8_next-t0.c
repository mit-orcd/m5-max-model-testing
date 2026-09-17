#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (s == NULL || n == 0) {
        return -1;
    }
    uint32_t b = s[0];
    if (b < 0x80) {
        *cp = b;
        return 1;
    }
    int m;
    if (b < 0xC0) {
        return -1;
    } else if (b < 0xE0) {
        m = 2;
    } else if (b < 0xF0) {
        m = 3;
    } else if (b < 0xF8) {
        m = 4;
    } else {
        return -1;
    }
    if (n < m) {
        return -1;
    }
    for (int i = 1; i < m; i++) {
        if ((s[i] & 0xC0) != 0x80) {
            return -1;
        }
    }
    uint32_t code_point = 0;
    switch (m) {
        case 2:
            code_point = ((b & 0x1F) << 6) | (s[1] & 0x3F);
            break;
        case 3:
            code_point = ((b & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
            break;
        case 4:
            code_point = ((b & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
            break;
        default:
            return -1;
    }
    if (m == 2) {
        if (code_point < 0x80 || code_point > 0x7FF) {
            return -1;
        }
    } else if (m == 3) {
        if (code_point < 0x800 || code_point > 0xFFFF) {
            return -1;
        }
    } else if (m == 4) {
        if (code_point < 0x10000 || code_point > 0x10FFFF) {
            return -1;
        }
    }
    if (code_point >= 0xD800 && code_point <= 0xDFFF) {
        return -1;
    }
    *cp = code_point;
    return m;
}