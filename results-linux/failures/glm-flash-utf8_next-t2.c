#include <stdint.h>
#include <stddef.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) {
        return -1;
    }

    const unsigned char *p = s;
    uint32_t result = 0;
    size_t bytes_needed = 0;
    size_t bytes_consumed = 0;

    // Determine the length of the first code point
    if ((p[0] & 0x80) == 0x00) {
        bytes_needed = 1;
    } else if ((p[0] & 0xE0) == 0xC0) {
        bytes_needed = 2;
    } else if ((p[0] & 0xF0) == 0xE0) {
        bytes_needed = 3;
    } else if ((p[0] & 0xF8) == 0xF0) {
        bytes_needed = 4;
    } else {
        return -1;
    }

    // Check for overlong encoding or invalid sequence length based on n
    if (n < bytes_needed) {
        return -1;
    }

    // Read bytes
    result = (uint32_t)p[0] & ((1 << (7 - bytes_needed)) - 1);
    bytes_consumed = 1;

    for (size_t i = 1; i < bytes_needed; i++) {
        if ((p[i] & 0xC0) != 0x80) {
            return -1; // Bad continuation byte
        }
        result = (result << 6) | (p[i] & 0x3F);
        bytes_consumed++;
    }

    // Validate Overlong Encoding and Max Code Point
    if (bytes_needed == 2) {
        // Overlong check for 2-byte sequences
        if (result < 0x80) return -1;
    } else if (bytes_needed == 3) {
        // Overlong check for 3-byte sequences
        if (result < 0x800) return -1;
        // Surrogate check
        if (result >= 0xD800 && result <= 0xDFFF) return -1;
    } else if (bytes_needed == 4) {
        // Max code point U+10FFFF
        if (result > 0x10FFFF) return -1;
    }

    *cp = result;
    return (int)bytes_consumed;
}