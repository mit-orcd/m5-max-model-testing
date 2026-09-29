#include <stdint.h>
#include <stddef.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char lead = s[0];
    size_t len;
    uint32_t code_point;

    if ((lead & 0x80) == 0x00) {
        len = 1;
        code_point = lead;
    } else if ((lead & 0xE0) == 0xC0) {
        len = 2;
        code_point = lead & 0x1F;
    } else if ((lead & 0xF0) == 0xE0) {
        len = 3;
        code_point = lead & 0x0F;
    } else if ((lead & 0xF8) == 0xF0) {
        len = 4;
        code_point = lead & 0x07;
    } else {
        return -1;
    }

    if (len > n) return -1;

    for (size_t i = 1; i < len; ++i) {
        if ((s[i] & 0xC0) != 0x80) return -1;
        code_point = (code_point << 6) | (s[i] & 0x3F);
    }

    if (len == 2 && code_point < 0x80) return -1;
    if (len == 3 && code_point < 0x800) return -1;
    if (len == 4 && code_point < 0x10000) return -1;
    if (code_point > 0x10FFFF) return -1;
    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;

    if (len == 2 && code_point != (lead & 0x1F)) return -1;
    if (len == 3 && code_point != (lead & 0x0F)) return -1;
    if (len == 4 && code_point != (lead & 0x07)) return -1;

    *cp = code_point;
    return (int)len;
}