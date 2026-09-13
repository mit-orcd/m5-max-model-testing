```c
#include <stdint.h>
#include <stddef.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    uint8_t b0 = s[0];
    if (b0 < 0x80) {
        // 1-byte character
        *cp = b0;
        return 1;
    } else if ((b0 & 0xE0) == 0xC0) {
        // 2-byte character
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        uint32_t cp_val = ((b0 & 0x1F) << 6) | (s[1] & 0x3F);
        if (cp_val < 0x80) return -1; // overlong
        *cp = cp_val;
        return 2;
    } else if ((b0 & 0xF0) == 0xE0) {
        // 3-byte character
        if (n < 3) return -1;