#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    
    uint32_t code_point = 0;
    unsigned char lead = s[0];
    unsigned char mask = 0x80;
    int bytes = 1;
    
    if (lead == 0) return -1; /* null byte is invalid */
    
    while ((mask & lead) != 0) {
        mask >>= 1;
        bytes++;
        if (bytes > 4) return -1;
    }
    
    if (bytes == 1) {
        *cp = lead & 0x7F;
        return 1;
    }
    
    if (bytes == 2) {
        if (n < 2) return -1;
        if ((s[0] & 0xE0) == 0xC0) {
            code_point = (s[0] & 0x1F) << 6;
        } else {
            return -1;
        }
        if ((s[1] & 0x80) == 0) return -1;
        if ((s[1] & 0x40) != 0) return -1;
        code_point |= (s[1] & 0x3F);
        if (code_point < 0x80) return -1;
        if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
        if (code_point > 0xFFFF) return -1;
    } else if (bytes == 3) {
        if (n < 3) return -1;
        if ((s[0] & 0xF0) == 0xE0) {
            code_point = (s[0] & 0xFF) << 12;
        } else {
            return -1;
        }
        if ((s[1] & 0x80) == 0) return -1;
        if ((s[1] & 0x40) == 0) return -1;
        code_point |= (s[1] & 0x3F) << 6;
        if ((s[2] & 0x80) == 0) return -1;
        if ((s[2] & 0x40) == 0) return -1;
        code_point |= (s[2] & 0x3F);
        if (code_point < 0x800) return -1;
        if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
        if (code_point > 0xFFFF) return -1;
    } else if (bytes == 4) {
        if (n < 4) return -1;
        if ((s[0] & 0xF8) == 0xF0) {
            code_point = (s[0] & 0xFF) << 18;
        } else {
            return -1;
        }
        if ((s[1] & 0x80) == 0) return -1;
        if ((s[1] & 0x40) == 0) return -1;
        code_point |= (s[1] & 0x3F) << 12;
        if ((s[2] & 0x80) == 0) return -1;
        if ((s[2] & 0x40) == 0) return -1;
        code_point |= (s[2] & 0x3F) << 6;
        if ((s[3] & 0x80) == 0) return -1;
        if ((s[3] & 0x40) == 0) return -1;
        code_point |= (s[3] & 0x3F);
        if (code_point < 0x10000) return -1;
        if (code_point > 0x10FFFF) return -1;
    } else {
        return -1;
    }
    
    *cp = code_point;
    return bytes;
}