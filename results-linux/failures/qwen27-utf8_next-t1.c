#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c0 = s[0];

    /* Continuation byte in leading position: 10xxxxxx */
    if ((c0 & 0xC0) == 0x80) return -1;

    /* 1-byte: 0xxxxxxx */
    if ((c0 & 0x80) == 0) {
        if (n < 1) return -1;
        *cp = (uint32_t)c0;
        return 1;
    }

    /* Determine expected length and initial code point bits */
    unsigned len;
    uint32_t ch;

    if ((c0 & 0xE0) == 0xC0) {
        len = 2;
        ch = (uint32_t)(c0 & 0x1F);
    } else if ((c0 & 0xF0) == 0xE0) {
        len = 3;
        ch = (uint32_t)(c0 & 0x0F);
    } else if ((c0 & 0xF8) == 0xF0) {
        len = 4;
        ch = (uint32_t)(c0 & 0x07);
    } else {
        /* 5-byte (0xF8), 6-byte (0xFC), or other invalid leading bytes */
        return -1;
    }

    if (n < len) return -1;

    for (unsigned i = 1; i < len; i++) {
        unsigned char c = s[i];
        if ((c & 0xC0) != 0x80) return -1;
        ch = (ch << 6) | (uint32_t)(c & 0x3F);
    }

    /* Reject overlong encodings */
    if (len == 2 && ch < 0x80) return -1;
    if (len == 3 && ch < 0x800) return -1;
    if (len == 4 && ch < 0x10000) return -1;

    /* Reject UTF-16 surrogates */
    if (ch >= 0xD800 && ch <= 0xDFFF) return -1;

    /* Reject anything above U+10FFFF */
    if (ch > 0x10FFFF) return -1;

    *cp = ch;
    return (int)len;
}