#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp)
{
    if (n == 0)
        return -1;

    uint32_t c0 = s[0];
    uint32_t expected_cont;
    uint32_t min_cp;
    int len;

    if (c0 < 0x80) {
        len = 1;
        expected_cont = 0;
        min_cp = 0;
    } else if (c0 >= 0xC0 && c0 < 0xE0) {
        len = 2;
        min_cp = 0x80;
        expected_cont = 0x80;
    } else if (c0 >= 0xE0 && c0 < 0xF0) {
        len = 3;
        min_cp = 0x800;
        expected_cont = 0x80;
    } else if (c0 >= 0xF0 && c0 < 0xF8) {
        len = 4;
        min_cp = 0x10000;
        expected_cont = 0x80;
    } else {
        return -1;
    }

    if (n < (size_t)len)
        return -1;

    uint32_t codepoint = c0 & (0xFFu >> len);
    for (int i = 1; i < len; i++) {
        uint32_t cb = s[i];
        if ((cb & 0xC0) != expected_cont)
            return -1;
        codepoint = (codepoint << 6) | (cb & 0x3F);
    }

    if (codepoint < min_cp)
        return -1;
    if (codepoint > 0x10FFFF)
        return -1;
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
        return -1;

    *cp = codepoint;
    return len;
}