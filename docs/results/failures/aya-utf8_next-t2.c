#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s[0] > 0x7f) return -1;
    uint32_t c = s[0];
    if (c < 0x80) {
        *cp = c;
        return 1;
    }
    if (n < 2 || (c = (s[1] & 0x3f) | (c & 0x80) << 6) > 0x7ff) return -1;
    *cp = c;
    return 2;
}