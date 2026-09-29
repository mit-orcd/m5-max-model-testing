#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) {
        return -1;
    }

    uint8_t first = s[0];
    uint32_t value = 0;
    int bytes_needed = 0;

    // Determine the number of bytes and the initial value based on the first byte
    if ((first & 0x80) == 0) {
        // 0xxxxxxx (ASCII)
        bytes_needed = 1;
        value = first;
    } else if ((first & 0xE0) == 0xC0) {
        // 110xxxxx (2-byte)
        bytes_needed = 2;
        value = first & 0x1F;
    } else if ((first & 0xF0) == 0xE0) {
        // 1110xxxx (3-byte)
        bytes_needed = 3;
        value = first & 0x0F;
    } else if ((first & 0xF8) == 0xF0) {
        // 11110xxx (4-byte)
        bytes_needed = 4;
        value = first & 0x07;
    } else {
        // Invalid leading byte (continuation byte or 5/6 byte form)
        return -1;
    }

    // Check for overlong encoding
    if (bytes_needed > 1) {
        uint32_t min_val;
        if (bytes_needed == 2) {
            min_val = 0x80; // U+0080
        } else if (bytes_needed == 3) {
            min_val = 0x800; // U+0800
        } else {
            min_val = 0x10000; // U+10000
        }
        if (value < min_val) {
            return -1;
        }
    }

    // Check for code point limits
    if (value > 0x10FFFF) {
        return -1;
    }

    // Check for UTF-16 surrogate halves
    if (value >= 0xD800 && value <= 0xDFFF) {
        return -1;
    }

    // Check if we have enough bytes in the input buffer
    if (n < (size_t)bytes_needed) {
        return -1;
    }

    // Decode continuation bytes
    for (int i = 1; i < bytes_needed; i++) {
        uint8_t cont = s[i];
        if ((cont & 0xC0) != 0x80) {
            // Invalid continuation byte
            return -1;
        }
        value = (value << 6) | (cont & 0x3F);
    }

    // Write the result
    *cp = value;
    return bytes_needed;
}