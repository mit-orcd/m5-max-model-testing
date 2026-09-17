#include <stddef.h>
#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    unsigned char b0 = s[0];
    if (b0 < 0x80) { *cp = b0; return 1; }
    if (b0 < 0xC2) return -1;              /* continuation in lead, or overlong C0/C1 */
    int len;
    uint32_t v, lo;
    if (b0 < 0xE0)      { len = 2; v = b0 & 0x1F; lo = 0x80; }
    else if (b0 < 0xF0) { len = 3; v = b0 & 0x0F; lo = 0x800; }
    else if (b0 < 0xF5) { len = 4; v = b0 & 0x07; lo = 0x10000; }
    else return -1;                        /* 5/6-byte forms and > U+10FFFF leads */
    if (n < (size_t)len) return -1;        /* truncated */
    for (int i = 1; i < len; i++) {
        unsigned char b = s[i];
        if ((b & 0xC0) != 0x80) return -1; /* bad continuation */
        v = (v << 6) | (b & 0x3F);
    }
    if (v < lo) return -1;                 /* overlong */
    if (v >= 0xD800 && v <= 0xDFFF) return -1;  /* surrogates */
    if (v > 0x10FFFF) return -1;
    *cp = v;
    return len;
}
