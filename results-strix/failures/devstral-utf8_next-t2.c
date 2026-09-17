#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t code_point = 0;
    int bytes_consumed = 0;
    unsigned char c = s[0];

    if ((c & 0x80) == 0x00) { // 1-byte sequence (0xxxxxxx)
        code_point = c;
        bytes_consumed = 1;
    } else if ((c & 0xE0) == 0xC0) { // 2-byte sequence (110xxxxx)
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        code_point = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        bytes_consumed = 2;
    } else if ((c & 0xF0) == 0xE0) { // 3-byte sequence (1110xxxx)
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return -1;
        code_point = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        bytes_consumed = 3;
    } else if ((c & 0xF8) == 0xF0) { // 4-byte sequence (11110xxx)
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return -1;
        code_point = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        bytes_consumed = 4;
    } else {
        return -1;
    }

    // Check for overlong encodings
    if (code_point <= 0x7F && bytes_consumed != 1) return -1;
    if (code_point <= 0x7FF && bytes_consumed != 2) return -1;
    if (code_point <= 0xFFFF && bytes_consumed != 3) return -1;

    // Check for surrogate pairs
    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;

    // Check for code points above U+10FFFF
    if (code_point > 0x10FFFF) return -1;

    *cp = code_point;
    return bytes_consumed;
}