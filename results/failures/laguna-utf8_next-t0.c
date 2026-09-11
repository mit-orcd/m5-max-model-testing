#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    
    unsigned char byte = s[0];
    int len;
    uint32_t codepoint;
    
    if ((byte & 0x80) == 0) {
        len = 1;
        codepoint = byte;
    } else if ((byte & 0xE0) == 0xC0) {
        len = 2;
        codepoint = byte & 0x1F;
    } else if ((byte & 0xF0) == 0xE0) {
        len = 3;
        codepoint = byte & 0x0F;
    } else if ((byte & 0xF8) == 0xF0) {
        len = 4;
        codepoint = byte & 0x07;
    } else {
        return -1;
    }
    
    if (len > (int)n) return -1;
    
    for (int i = 1; i < len; i++) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        codepoint = (codepoint << 6) | (s[i] & 0x3F);
    }
    
    if (len == 2) {
        if (codepoint < 0x80) return -1;
    } else if (len == 3) {
        if (codepoint < 0x800) return -1;
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;
    } else if (len == 4) {
        if (codepoint < 0x10000) return -1;
        if (codepoint > 0x10FFFF) return -1;
    }
    
    *cp = codepoint;
    return len;
}