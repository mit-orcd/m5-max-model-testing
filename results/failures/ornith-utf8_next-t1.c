#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t c0 = s[0];
    uint32_t cp_out;
    size_t len;

    if (c0 < 0x80) {
        cp_out = c0;
        len = 1;
    } else if ((c0 & 0xE0) == 0xC0) {
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        /* reject overlong: U+0080..U+007F uses 2 bytes */
        cp_out = (c0 & 0x1F) << 6 | (s[1] & 0x3F);
        if (cp_out < 0x80) return -1;
        len = 2;
    } else if ((c0 & 0xF0) == 0xE0) {
        if (n < 3) return -1;
        if (((s[1] & 0xC0) != 0x80) || ((s[2] & 0xC0) != 0x80)) return -1;
        cp_out = (c0 & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
        if (cp_out < 0x800) return -1;
        len = 3;
    } else if ((c0 & 0xF8) == 0xF0) {
        if (n < 4) return -1;
        if (!(((s[1] & 0xC0) == 0x80) && ((s[2] & 0xC0) == 0x80) &&
              ((s[3] & 0xC0) == 0x80))) return -1;
        cp_out = (c0 & 0x07) << 18 | (s[1] & 0x3F) << 12 |
                 (s[2] & 0x3F) << 6 | (s[3] & 0x3F);
        if (cp_out < 0x10000) return -1;
        if (cp_out > 0x10FFFF) return -1;
        len = 4;
    } else {
        /* leading byte is 0x80-0xBF (continuation), 0xF1-0xF7 (5/6 byte),
           or 0xC0/0xC1 (overlong 2-byte) — all invalid */
        return -1;
    }

    /* reject UTF-16 surrogates */
    if (cp_out >= 0xD800 && cp_out <= 0xDFFF) return -1;

    *cp = cp_out;
    return (int)len;
}