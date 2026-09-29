#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char c = s[0];
    int len = 0;
    uint32_t value = 0;

    // Determine expected length and validate leading byte
    if ((c & 0x80) == 0) {
        len = 1;
        value = c;
    } else if ((c & 0xE0) == 0xC0) {
        len = 2;
        value = c & 0x1F;
    } else if ((c & 0xF0) == 0xE0) {
        len = 3;
        value = c & 0x0F;
    } else if ((c & 0xF8) == 0xF0) {
        len = 4;
        value = c & 0x07;
    } else if ((c & 0xFC) == 0xF8) {
        // Obsolete 5-byte form
        return -1;
    } else if ((c & 0xFE) == 0xFC) {
        // Obsolete 6-byte form
        return -1;
    } else {
        // Continuation byte in leading position or invalid leading byte
        return -1;
    }

    if (n < (size_t)len) {
        // Truncated sequence
        return -1;
    }

    // Validate and accumulate continuation bytes
    for (int i = 1; i < len; i++) {
        unsigned char cont = s[i];
        if ((cont & 0xC0) != 0x80) {
            // Bad continuation byte
            return -1;
        }
        value = (value << 6) | (cont & 0x3F);
    }

    // Check for overlong encodings
    if (len == 2 && value < 0x80) return -1;
    if (len == 3 && value < 0x800) return -1;
    if (len == 4 && value < 0x10000) return -1;

    // Check for surrogates (U+D800 to U+DFFF)
    if (value >= 0xD800 && value <= 0xDFFF) return -1;

    // Check for code points above U+10FFFF
    if (value > 0x10FFFF) return -1;

    *cp = value;
    return len;
}