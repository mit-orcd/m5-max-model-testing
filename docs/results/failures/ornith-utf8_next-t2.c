#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) {
        return -1;
    }

    uint32_t cp_val;
    int num_bytes;
    size_t i;

    if ((s[0] & 0x80) == 0x00) {
        /* 1-byte sequence: 0xxxxxxx */
        if (n < 1) {
            return -1;
        }
        cp_val = s[0];
        num_bytes = 1;
    } else if ((s[0] & 0xe0) == 0xc0) {
        /* 2-byte sequence: 110xxxxx 10xxxxxx */
        if (n < 2) {
            return -1;
        }
        if ((s[1] & 0xc0) != 0x80) {
            return -1;
        }
        cp_val = (uint32_t)(s[0] & 0x1f) << 6
               | (uint32_t)(s[1] & 0x3f);
        /* Reject overlong (U+0000..U+007F must use 1 byte) */
        if (cp_val < 0x80) {
            return -1;
        }
        num_bytes = 2;
    } else if ((s[0] & 0xf0) == 0xe0) {
        /* 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx */
        if (n < 3) {
            return -1;
        }
        for (i = 1; i < 3; i++) {
            if ((s[i] & 0xc0) != 0x80) {
                return -1;
            }
        }
        cp_val = (uint32_t)(s[0] & 0x0f) << 12
               | (uint32_t)(s[1] & 0x3f) << 6
               | (uint32_t)(s[2] & 0x3f);
        /* Reject overlong (U+0080..U+07FF must use at most 2 bytes) */
        if (cp_val < 0x80) {
            return -1;
        }
        /* Reject surrogate code points */
        if (cp_val >= 0xD800 && cp_val <= 0xDFFF) {
            return -1;
        }
        num_bytes = 3;
    } else if ((s[0] & 0xf8) == 0xf0) {
        /* 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        if (n < 4) {
            return -1;
        }
        for (i = 1; i < 4; i++) {
            if ((s[i] & 0xc0) != 0x80) {
                return -1;
            }
        }
        cp_val = (uint32_t)(s[0] & 0x07) << 18
               | (uint32_t)(s[1] & 0x3f) << 12
               | (uint32_t)(s[2] & 0x3f) << 6
               | (uint32_t)(s[3] & 0x3f);
        /* Reject overlong (U+0800..U+FFFF must use at most 3 bytes) */
        if (cp_val < 0x800) {
            return -1;
        }
        /* Reject surrogates */
        if (cp_val >= 0xD800 && cp_val <= 0xDFFF) {
            return -1;
        }
        /* Reject above maximum code point */
        if (cp_val > 0x10FFFF) {
            return -1;
        }
        num_bytes = 4;
    } else {
        /* Continuation byte (10xxxxxx) in leading position,
         * or invalid lead byte (111110xx / 111111xx / 1xxxxxxx).
         * These correspond to the obsolete 5- and 6-byte forms
         * and other malformed lead bytes. */
        return -1;
    }

    *cp = cp_val;
    return num_bytes;
}