#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) {
        return -1;
    }

    uint8_t first = s[0];
    uint32_t value = 0;
    size_t length = 0;

    // Determine length and basic validity
    if ((first & 0x80) == 0x00) {
        // 0xxxxxxx
        length = 1;
        value = first;
    } else if ((first & 0xE0) == 0xC0) {
        // 110xxxxx
        length = 2;
        value = first & 0x1F;
    } else if ((first & 0xF0) == 0xE0) {
        // 1110xxxx
        length = 3;
        value = first & 0x0F;
    } else if ((first & 0xF8) == 0xF0) {
        // 11110xxx
        length = 4;
        value = first & 0x07;
    } else {
        // Invalid leading byte (continuation byte or 5/6 byte form)
        return -1;
    }

    // Check for overlong encoding
    if (length > 1) {
        uint32_t min_val;
        switch (length) {
            case 2: min_val = 0x80; break;
            case 3: min_val = 0x800; break;
            case 4: min_val = 0x10000; break;
            default: return -1;
        }
        if (value < min_val) {
            return -1;
        }
    }

    // Check for code point > U+10FFFF
    if (value > 0x10FFFF) {
        return -1;
    }

    // Check for UTF-16 surrogate range
    if (value >= 0xD800 && value <= 0xDFFF) {
        return -1;
    }

    // Check for truncation
    if (length > n) {
        return -1;
    }

    // Decode continuation bytes
    for (size_t i = 1; i < length; i++) {
        if ((s[i] & 0xC0) != 0x80) {
            return -1; // Bad continuation byte
        }
        value = (value << 6) | (s[i] & 0x3F);
    }

    *cp = value;
    return (int)length;
}