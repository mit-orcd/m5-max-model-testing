#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    
    uint32_t code_point = 0;
    unsigned char lead = s[0];
    
    if (lead < 0x80) {
        *cp = lead;
        return 1;
    }
    
    unsigned char mask = 0x3f;
    unsigned char prefix = 0x80;
    unsigned char mask2 = 0x7f;
    
    if ((lead & 0xc0) == 0x80) return -1; /* continuation byte in leading position */
    
    unsigned char byte1 = lead & 0x7f;
    unsigned char byte2 = lead & 0x3f; /* for checking overlong encoding */
    
    if ((lead & 0xe0) == 0xc0) {
        if (n < 2) return -1;
        if ((s[1] & 0xc0) != 0x80) return -1;
        if ((s[1] & 0x80) == 0x00) return -1;
        code_point = (byte1 << 6) | (s[1] & 0x3f);
        if (code_point < 0x80) return -1; /* overlong encoding */
        if (0xD800 <= code_point && code_point <= 0xDFFF) return -1; /* surrogate */
        if (code_point > 0x7FF) return -1; /* overlong encoding */
        return 2;
    }
    
    if ((lead & 0xf0) == 0xe0) {
        if (n < 3) return -1;
        if ((s[1] & 0xc0) != 0x80) return -1;
        if ((s[2] & 0xc0) != 0x80) return -1;
        code_point = (byte1 << 12) | ((s[1] & 0x3f) << 6) | (s[2] & 0x3f);
        if (code_point < 0x800) return -1; /* overlong encoding */
        if (0xD800 <= code_point && code_point <= 0xDFFF) return -1; /* surrogate */
        if (code_point > 0xFFFF) return -1; /* above U+10FFFF */
        return 3;
    }
    
    if ((lead & 0xf8) == 0xf0) {
        if (n < 4) return -1;
        if ((s[1] & 0xc0) != 0x80) return -1;
        if ((s[2] & 0xc0) != 0x80) return -1;
        if ((s[3] & 0xc0) != 0x80) return -1;
        code_point = (byte1 << 18) | ((s[1] & 0x3f) << 12) | ((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
        if (code_point > 0x10FFFF || code_point < 0x10000) return -1; /* overlong encoding */
        return 4;
    }
    
    return -1;
}