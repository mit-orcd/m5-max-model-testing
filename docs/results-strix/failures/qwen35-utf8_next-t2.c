#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t c = s[0];
    int bytes = 0;
    uint32_t mask = 0;
    uint32_t continuation_mask = 0x80;
    uint32_t continuation_value = 0x80;

    if ((c & 0x80) == 0) {
        // 1-byte
        bytes = 1;
        *cp = c;
        return 1;
    }

    if ((c & 0xE0) == 0xC0) {
        bytes = 2;
        mask = 0x1F;
        *cp = c & mask;
    } else if ((c & 0xF0) == 0xE0) {
        bytes = 3;
        mask = 0x0F;
        *cp = c & mask;
    } else if ((c & 0xF8) == 0xF0) {
        bytes = 4;
        mask = 0x07;
        *cp = c & mask;
    } else {
        // Invalid start byte (10xxxxxx or 11111xxx)
        return -1;
    }

    if (n < (size_t)bytes) {
        return -1; // Truncated
    }

    // Decode continuation bytes and check validity
    for (int i = 1; i < bytes; ++i) {
        if ((s[i] & 0xC0) != 0x80) {
            return -1; // Invalid continuation
        }
        *cp = (*cp << 6) | (s[i] & 0x3F);
    }

    // Check for overlong encodings
    // 2-byte: must be >= 0x80. If < 0x80, it's overlong.
    if (bytes == 2 && *cp < 0x80) return -1;
    // 3-byte: must be >= 0x800. If < 0x800, it's overlong.
    if (bytes == 3 && *cp < 0x800) return -1;
    // 4-byte: must be >= 0x10000. If < 0x10000, it's overlong.
    if (bytes == 4 && *cp < 0x10000) return -1;

    // Check for surrogates (U+D800 to U+DFFF)
    if (*cp >= 0xD800 && *cp <= 0xDFFF) return -1;

    // Check for code points above U+10FFFF
    if (*cp > 0x10FFFF) return -1;

    return bytes;
}