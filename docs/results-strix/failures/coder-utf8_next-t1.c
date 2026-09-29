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
    
    uint32_t code_point = c & (0x7F >> len);
    
    for (int i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        code_point = (code_point << 6) | (s[i] & 0x3F);
    }
    
    if (code_point > 0x10FFFF) return -1;
    
    switch (len) {
        case 2:
            if (code_point < 0x80) return -1;
            break;
        case 3:
            if (code_point < 0x800) return -1;
            if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
            break;
        case 4:
            if (code_point < 0x10000) return -1;
            break;
    }
    
    *cp = code_point;
    return len;
}