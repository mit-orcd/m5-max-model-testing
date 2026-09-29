#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0)
        return -1;

    uint32_t c = s[0];
    uint32_t code_point;
    size_t extra_bytes;
    int num_bytes;

    if (c < 0x80) {
        num_bytes = 1;
        code_point = c;
    } else if ((c & 0xE0) == 0xC0) {
        num_bytes = 2;
        extra_bytes = 1;
        if (n < (size_t)num_bytes)
            return -1;
        code_point = c & 0x1F;
    } else if ((c & 0xF0) == 0xE0) {
        num_bytes = 3;
        extra_bytes = 2;
        if (n < (size_t)num_bytes)
            return -1;
        code_point = c & 0x0F;
    } else if ((c & 0xF8) == 0xE0 & 0xF8) {
        return -1;
    } else if ((c & 0xF8) == 0xF0) {
        num_bytes = 4;
        extra_bytes = 3;
        if (n < (size_t)num_bytes)
            return -1;
        code_point = c & 0x07;
    } else {
        return -1;
    }

    for (size_t i = 1; i <= extra_bytes; ++i) {
        uint32_t cont = s[i];
        if ((cont & 0xC0) != 0x80)
            return -1;
        code_point = (code_point << 6) | (cont & 0x3F);
    }

    if (num_bytes == 2 && code_point < 0x80)
        return -1;
    if (num_bytes == 3 && code_point < 0x800)
        return -1;
    if (num_bytes == 4 && code_point < 0x10000)
        return -1;

    if (code_point > 0x10FFFF)
        return -1;
    if (code_point >= 0xD800 && code_point <= 0xDFFF)
        return -1;

    *cp = code_point;
    return num_bytes;
}