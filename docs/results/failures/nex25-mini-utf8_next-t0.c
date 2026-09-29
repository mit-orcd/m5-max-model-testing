#include <stdint.h>
#include <stddef.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp)
{
    uint32_t code;
    size_t length, i, minimum;
    unsigned char c;

    if (s == NULL || cp == NULL || n == 0)
        return -1;

    c = s[0];

    if (c < 0x80) {
        length = 1;
        minimum = 0x80;
        code = c;
    } else if (c < 0xE0) {
        length = 2;
        minimum = 0x80;
        code = c & 0x1F;
    } else if (c < 0xF0) {
        length = 3;
        minimum = 0x800;
        code = c & 0x0F;
    } else if (c < 0xF8) {
        length = 4;
        minimum = 0x10000;
        code = c & 0x07;
    } else {
        return -1;
    }

    if (n < length)
        return -1;

    for (i = 1; i < length; ++i) {
        if ((s[i] & 0xC0) != 0x80)
            return -1;
        code = (code << 6) | (s[i] & 0x3F);
    }

    if (code < minimum || code > 0x10FFFF ||
        (code >= 0xD800 && code <= 0xDFFF))
        return -1;

    *cp = code;
    return (int)length;
}