#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    const unsigned char *s_end = s + n;
    const unsigned char *p = s;

    // Check for leading byte (0xxxxxxx)
    if ((*p & 0x80) == 0) {
        *cp = *p;
        return 1;
    }

    // Determine the number of bytes in the sequence
    int len;
    if ((*p & 0xE0) == 0xC0) {
        len = 2;
    } else if ((*p & 0xF0) == 0xE0) {
        len = 3;
    } else if ((*p & 0xF8) == 0xF0) {
        len = 4;
    } else {
        return -1;
    }

    // Validate sequence length against available bytes
    if ((size_t)(p + len) > s_end) {
        return -1;
    }

    // Decode the code point
    uint32_t value = *p & ((1 << (8 - len)) - 1);

    // Process continuation bytes
    for (int i = 1; i < len; i++) {
        unsigned char c = p[i];
        // Check for invalid continuation byte (must start with 10xxxxxx)
        if ((c & 0xC0) != 0x80) {
            return -1;
        }
        value = (value << 6) | (c & 0x3F);
    }

    // Overlong check: Ensure the sequence uses the minimum number of bytes
    uint32_t min_val;
    if (len == 2) min_val = 0x80;
    else if (len == 3) min_val = 0x800;
    else if (len == 4) min_val = 0x10000;
    else return -1; // Should not happen

    if (value < min_val) {
        return -1;
    }

    // Surrogate check: Reject U+D800 to U+DFFF
    if ((value & 0xFFFFF800) == 0xD800) {
        return -1;
    }

    // Max code point check: Reject anything above U+10FFFF
    if (value > 0x10FFFF) {
        return -1;
    }

    *cp = value;
    return len;
}