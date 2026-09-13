#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s[0] > 0x7f) return -1;
    uint32_t code = s[0];
    if (code < 0x80) {
        *cp = code;
        return 1;
    }
    if (n < 2 || (code = (s[1] & 0x3f) | (code & 0x7f) << 6) > 0x7ff) return -1;
    if ((code & 0xf800) == 0xd800) return -1;
    *cp = code;
    return 2;
}