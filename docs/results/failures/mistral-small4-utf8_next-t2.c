#include <stdint.h>
#include <stdbool.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s == NULL || cp == NULL) {
        return -1;
    }

    unsigned char c = s[0];
    if (c < 0x80) {
        *cp = c;
        return 1;
    }

    int len;
    uint32_t codepoint;

    if ((c & 0xE0) == 0xC0) {
        len = 2;
        codepoint = c & 0x1F;
    } else if ((c & 0xF0) == 0xE0) {
        len = 3;
        codepoint = c & 0x0F;
    } else if ((c & 0xF8) == 0xF0) {
        len = 4;
        codepoint = c & 0x07;
    } else {
        return -1;
    }

    if (len > n) {
        return -1;
    }

    for (int i = 1; i < len; i++) {
        unsigned char c = s[i];
        if ((c & 0xC0) != 0x80) {
            return -1;
        }
        codepoint = (codepoint << 6) | (c & 0x3F);
    }

    if (len == 2) {
        if (codepoint < 0x80) {
            return -1;
        }
    } else if (len == 3) {
        if (codepoint < 0x800) {
            return -1;
        }
    } else if (len == 4) {
        if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
            return -1;
        }
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
            return -1;
        }
    }

    *cp = codepoint;
    return len;
}