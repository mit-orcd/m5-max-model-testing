#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    
    uint32_t c = s[0];
    int len;
    
    if ((c & 0x80) == 0) {
        *cp = c;
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        len = 2;
    } else if ((c & 0xF0) == 0xE0) {
        len = 3;
    } else if ((c & 0xF8) == 0xF0) {
        len = 4;
    } else {
        return -1;
    }
    
    if (len > n) return -1;
    
    if (len == 2) {
        if ((s[1] & 0xC0) != 0x80) return -1;
        c = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        if (c < 0x80) return -1;
    } else if (len == 3) {
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        c = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        if (c < 0x800) return -1;
        if (c >= 0xD800 && c <= 0xDFFF) return -1;
    } else {
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        c = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        if (c < 0x10000) return -1;
        if (c > 0x10FFFF) return -1;
    }
    
    *cp = c;
    return len;
}