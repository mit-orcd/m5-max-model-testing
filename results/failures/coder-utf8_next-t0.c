#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    
    uint32_t code_point = 0;
    int bytes = 1;
    
    if ((s[0] & 0x80) == 0) {
        code_point = s[0];
    } else if ((s[0] & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        if ((s[1] & 0x80) == 0 || (s[1] & 0x40) != 0) return -1;
        code_point = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        if (code_point < 0x80) return -1;
        if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
        if (code_point > 0xFFFF) return -1;
        bytes = 2;
    } else if ((s[0] & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        if ((s[1] & 0x80) == 0 || (s[1] & 0x40) != 0) return -1;
        if ((s[2] & 0x80) == 0 || (s[2] & 0x40) != 0) return -1;
        code_point = ((s[0] & 0xF) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        if (code_point < 0x800) return -1;
        if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
        if (code_point > 0xFFFF) return -1;
        bytes = 3;
    } else if ((s[0] & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        if ((s[1] & 0x80) == 0 || (s[1] & 0x40) != 0) return -1;
        if ((s[2] & 0x80) == 0 || (s[2] & 0x40) != 0) return -1;
        if ((s[3] & 0x80) == 0 || (s[3] & 0x40) != 0) return -1;
        code_point = ((s[0] & 0x7) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        if (code_point < 0x10000) return -1;
        if (code_point > 0x10FFFF) return -1;
        bytes = 4;
    } else {
        return -1;
    }
    
    *cp = code_point;
    return bytes;
}