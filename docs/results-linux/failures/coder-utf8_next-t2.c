#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    
    uint32_t code_point;
    unsigned char lead = s[0];
    int bytes;
    
    if ((lead & 0x80) == 0) {
        code_point = lead;
        bytes = 1;
    } else if ((lead & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        code_point = ((lead & 0x1F) << 6) | (s[1] & 0x3F);
        bytes = 2;
        if (code_point < 0x80) return -1; // overlong
    } else if ((lead & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        code_point = ((lead & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        bytes = 3;
        if (code_point < 0x800) return -1; // overlong
        if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1; // surrogate
        if (code_point > 0x10FFFF) return -1; // above maximum
    } else if ((lead & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        code_point = ((lead & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        bytes = 4;
        if (code_point < 0x10000) return -1; // overlong
        if (code_point > 0x10FFFF) return -1; // above maximum
    } else {
        return -1; // invalid leading byte
    }
    
    *cp = code_point;
    return bytes;
}