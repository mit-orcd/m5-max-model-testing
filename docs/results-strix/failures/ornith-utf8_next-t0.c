#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp)
{
    if (n == 0)
        return -1;

    uint32_t cpv = s[0];

    if (cpv < 0x80) {
        if (cpv < 0x20 && cpv == '\0')
            return -1;
        *cp = cpv;
        return 1;
    }

    if (cpv >= 0xF5 && cpv <= 0xFF)
        return -1;

    int extra;
    uint32_t lower, upper;
    if (cpv < 0xC0) {
        return -1;
    } else if (cpv < 0xE0) {
        extra = 1;
        lower = 0x80;
        upper = 0x7F;
    } else if (cpv < 0xF0) {
        extra = 2;
        lower = 0x800;
        upper = 0x7FF;
    } else {
        extra = 3;
        lower = 0x10000;
        upper = 0x10FFFF;
    }

    if ((size_t)extra > n - 1)
        return -1;

    cpv &= (1u << (8 - extra - 1)) - 1;
    for (int i = 1; i <= extra; i++) {
        unsigned char c = s[i];
        if (c < 0x80 || c > 0xBF)
            return -1;
        cpv = (cpv << 6) | (c & 0x3F);
    }

    if (cpv < lower || cpv > upper)
        return -1;
    if (cpv >= 0xD800 && cpv <= 0xDFFF)
        return -1;

    *cp = cpv;
    return extra + 1;
}