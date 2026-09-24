#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c0 = s[0];
    int len = 0;
    uint32_t code = 0;

    if (c0 < 0x80) {
        len = 1;
        code = c0;
    } else if ((c0 >> 5) == 0x6) { // 110xxxxx
        len = 2;
        code = c0 & 0x1F;
    } else if ((c0 >> 4) == 0xE) { // 1110xxxx
        len = 3;
        code = c0 & 0x0F;
    } else if ((c0 >> 3) == 0x1E) { // 11110xxx
        len = 4;
        code = c0 & 0x07;
    } else {
        return -1; // Invalid leading byte (continuation or 5/6-byte)
    }

    if (n < (size_t)len) return -1; // Truncated

    for (int i = 1; i < len; i++) {
        unsigned char c = s[i];
        if ((c & 0xC0) != 0x80) return -1; // Not a continuation byte
        code = (code << 6) | (c & 0x3F);
    }

    // Check for overlong encoding
    switch (len) {
        case 2:
            if (code < 0x80) return -1;
            break;
        case 3:
            if (code < 0x800) return -1;
            break;
        case 4:
            if (code < 0x10000) return -1;
            break;
    }

    // Check for surrogate range
    if (code >= 0xD800 && code <= 0xDFFF) return -1;

    // Check for beyond Unicode max
    if (code > 0x10FFFF) return -1;

    *cp = code;
    return len;
}