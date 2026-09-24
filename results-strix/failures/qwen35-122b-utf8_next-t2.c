#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c = s[0];
    uint32_t code_point;
    int len;

    if ((c & 0x80) == 0) {
        // 1-byte sequence: 0xxxxxxx
        code_point = c;
        len = 1;
    } else if ((c & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        code_point = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        len = 2;
        // Check for overlong encoding (must be >= 0x80)
        if (code_point < 0x80) return -1;
    } else if ((c & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        code_point = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        len = 3;
        // Check for overlong encoding (must be >= 0x800)
        if (code_point < 0x800) return -1;
        // Check for surrogates U+D800 to U+DFFF
        if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;
    } else if ((c & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        code_point = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        len = 4;
        // Check for overlong encoding (must be >= 0x10000)
        if (code_point < 0x10000) return -1;
        // Check for max code point U+10FFFF
        if (code_point > 0x10FFFF) return -1;
    } else if ((c & 0xFC) == 0xF8 || (c & 0xFE) == 0xFC) {
        // Obsolete 5-byte or 6-byte forms
        return -1;
    } else {
        // Continuation byte in leading position or invalid start byte
        return -1;
    }

    *cp = code_point;
    return len;
}