#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s[0] < 0x80) return -1;
    uint32_t code = s[0];
    size_t i = 1;
    if (code < 0x800) {
        if (code < 0x80) return -1;
        if (n < 2) return -1;
        if (s[1] < 0x80 || s[1] > 0xBF) return -1;
        code = (code << 6) | s[1];
    } else if (code < 0xD800) {
        if (n < 3) return -1;
        if (s[1] < 0x80 || s[1] > 0xBF) return -1;
        if (s[2] < 0x80 || s[2] > 0xBF) return -1;
        code = (code << 12) | (s[1] << 6) | s[2];
    } else {
        if (code >= 0xDC00 && code <= 0xDFFF) return -1;
        if (n < 4) return -1;
        if (s[1] < 0x80 || s[1] > 0xBF) return -1;
        if (s[2] < 0x80 || s[2] > 0xBF) return -1;
        if (s[3] < 0x80 || s[3] > 0xBF) return -1;
        code = (code << 18) | (s[1] << 12) | (s[2] << 6) | s[3];
    }
    if (code >= 0x100000) return -1;
    *cp = code;
    return i;
}