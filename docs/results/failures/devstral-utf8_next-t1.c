#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    unsigned char c = s[0];
    int len;

    if ((c & 0x80) == 0x00) {
        len = 1;
        *cp = c;
        if (c == 0) return -1; // NULL byte in middle
    } else if ((c & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((c & 0x1E) == 0) return -1; // overlong
        *cp = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        len = 2;
    } else if ((c & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        if ((c & 0x0F) == 0) return -1; // overlong
        *cp = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        if (*cp < 0x10000) return -1; // overlong
        if ((*cp & 0xFFFF0000) == 0xD8000000) return -1; // surrogate
        len = 3;
    } else if ((c & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        if ((c & 0x07) == 0) return -1; // overlong
        *cp = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        if (*cp > 0x10FFFF) return -1; // too large
        len = 4;
    } else {
        return -1;
    }

    return len;
}