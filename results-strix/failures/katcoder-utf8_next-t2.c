#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    uint32_t code_point;
    int bytes_needed;
    unsigned char first = s[0];

    if ((first & 0x80) == 0) {
        code_point = first;
        bytes_needed = 1;
    } else if ((first & 0xE0) == 0xC0) {
        code_point = first & 0x1F;
        bytes_needed = 2;
    } else if ((first & 0xF0) == 0xE0) {
        code_point = first & 0x0F;
        bytes_needed = 3;
    } else if ((first & 0xF8) == 0xF0) {
        code_point = first & 0x07;
        bytes_needed = 4;
    } else {
        return -1;
    }

    if (bytes_needed > (int)n) return -1;

    for (int i = 1; i < bytes_needed; i++) {
        unsigned char c = s[i];
        if ((c & 0xC0) != 0x80) return -1;
        code_point = (code_point << 6) | (c & 0x3F);
    }

    /* Reject overlong encodings */
    if (bytes_needed == 2 && code_point < 0x80) return -1;
    if (bytes_needed == 3 && code_point < 0x800) return -1;
    if (bytes_needed == 4 && code_point < 0x10000) return -1;

    /* Reject surrogates and values above U+10FFFF */
    if (code_point > 0x10FFFF) return -1;
    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;

    *cp = code_point;
    return bytes_needed;
}