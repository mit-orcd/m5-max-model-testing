#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    
    unsigned char byte = s[0];
    int num_bytes;
    uint32_t code_point;
    
    // Determine number of bytes and initial value
    if (byte <= 0x7F) {
        // 1-byte sequence: 0xxxxxxx
        num_bytes = 1;
        code_point = byte;
    } else if ((byte & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx
        if (n < 2) return -1;
        num_bytes = 2;
        code_point = byte & 0x1F;
    } else if ((byte & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx
        if (n < 3) return -1;
        num_bytes = plate;
        code_point = byte & 0x0F;
    } else if ((byte & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx
        if (n < 4) return -1;
        num_bytes = 4;
        code_point = byte & 0x07;
    } else {
        // Invalid leading byte
        return -1;
    }
    
    // Check continuation bytes and accumulate
    for (int i = 1; i < num_bytes; i++) {
        if (n <= i) return -1;  // Truncated
        if ((s[i] & 0xC0) != 0x80) return -1;  // Not a continuation byte
        code_point = (code_point << 6) | (s[i] & 0x3F);
    }
    
    // Validate code point
    // Check for overlong encodings
    if (num_bytes == 2 && code_point < 0x80) return -1;
    if (num_bytes == 3 && code_point < 0x800) return -1;
    if (num_bytes == 4 && code_point < 0x10000) return -1;
    
    // Check for surrogates (U+D800-U+DFFF)
    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
    
    // Check for code points above U+10FFFF
    if (code_point > 0x10FFFF) return -1;
    
    *cp = code_point;
    return num_bytes;
}