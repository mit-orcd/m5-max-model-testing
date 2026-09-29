#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char b0 = s[0];

    /* Continuation byte in leading position is invalid */
    if ((b0 & 0xC0) == 0x80) return -1;

    uint32_t codepoint;
    size_t len;
    unsigned char mask;
    uint32_t min_cp;

    if (b0 < 0x80) {
        /* 1-byte sequence: 0xxxxxxx */
        len = 1;
        codepoint = b0;
        min_cp = 0;
    } else if ((b0 & 0xE0) == 0xC0) {
        /* 2-byte sequence: 110xxxxx 10xxxxxx */
        if (n < 2) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        codepoint = ((uint32_t)(b0 & 0x1F) << 6) | (uint32_t)(s[1] & 0x3F);
        len = 2;
        min_cp = 0x80; /* reject overlong: cp < 0x80 */
    } else if ((b0 & 0xF0) == 0xE0) {
        /* 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx */
        if (n < 3) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        codepoint = ((uint32_t)(b0 & 0x0F) << 12)
                  | ((uint32_t)(s[1] & 0x3F) << 6)
                  | (uint32_t)(s[2] & 0x3F);
        len = 3;
        min_cp = 0x800; /* reject overlong: cp < 0x800 */
    } else if ((b0 & 0xF8) == 0xF0) {
        /* 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        if (n < 4) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        if ((s[3] & 0xC0) != 0x80) return -1;
        codepoint = ((uint32_t)(b0 & 0x07) << 18)
                  | ((uint32_t)(s[1] & 0x3F) << 12)
                  | ((uint32_t)(s[2] & 0x3F) << 6)
                  | (uint32_t)(s[3] & 0x3F);
        len = 4;
        min_cp = 0x10000; /* reject overlong: cp < 0x10000 */
    } else {
        /* 5- and 6-byte forms (0xF8-0xFF) are obsolete and invalid */
        return -1;
    }

    /* Reject overlong encodings */
    if (codepoint < min_cp) return -1;

    /* Reject UTF-16 surrogates */
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;

    /* Reject anything above U+10FFFF */
    if (codepoint > 0x10FFFF) return -1;

    *cp = codepoint;
    return (int)len;
}