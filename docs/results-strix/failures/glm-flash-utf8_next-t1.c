#include <stdint.h>

static inline int is_continuation_byte(uint8_t b) {
    return (b & 0xC0) == 0x80;
}

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0 || s == NULL || cp == NULL) {
        return -1;
    }

    uint8_t b = s[0];
    uint32_t result = 0;
    size_t bytes_needed = 0;
    size_t max_bytes = (n > 4) ? 4 : n;
    int is_overlong = 0;

    if ((b & 0x80) == 0x00) {
        /* 0xxxxxxx */
        result = b;
        bytes_needed = 1;
    } else if ((b & 0xE0) == 0xC0) {
        /* 110xxxxx */
        if (n < 2) return -1;
        result = b & 0x1F;
        bytes_needed = 2;
    } else if ((b & 0xF0) == 0xE0) {
        /* 1110xxxx */
        if (n < 3) return -1;
        result = b & 0x0F;
        bytes_needed = 3;
    } else if ((b & 0xF8) == 0xF0) {
        /* 11110xxx */
        if (n < 4) return -1;
        result = b & 0x07;
        bytes_needed = 4;
    } else {
        /* Invalid leading byte */
        return -1;
    }

    /* Check for overlong encoding */
    if (bytes_needed == 2 && result < 0x80) is_overlong = 1;
    if (bytes_needed == 3 && result < 0x800) is_overlong = 1;
    if (bytes_needed == 4 && result < 0x10000) is_overlong = 1;

    /* Validate continuation bytes */
    for (size_t i = 1; i < bytes_needed; i++) {
        if (i >= n) return -1;
        uint8_t cont = s[i];
        if (!is_continuation_byte(cont)) return -1;
        result = (result << 6) | (cont & 0x3F);
    }

    /* Check for invalid code points */
    if (is_overlong) return -1;
    if (result >= 0xD800 && result <= 0xDFFF) return -1;
    if (result > 0x10FFFF) return -1;

    *cp = result;
    return (int)bytes_needed;
}