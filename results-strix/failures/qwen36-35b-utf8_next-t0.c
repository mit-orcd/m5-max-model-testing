#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t code_point = 0;
    int bytes_needed = 0;
    int i;

    // Determine expected bytes based on first byte
    if ((s[0] & 0x80) == 0) {
        // 1-byte sequence: 0xxxxxxx
        code_point = s[0];
        bytes_needed = 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx
        bytes_needed = 2;
    } else if ((s[0] & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx
        bytes_needed = 3;
    } else if ((s[0] & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx
        bytes_needed = 4;
    } else {
        // Invalid leading byte (10xxxxxx or 11111xxx+)
        return -1;
    }

    // Check if we have enough bytes
    if (n < (size_t)bytes_needed) {
        return -1;
    }

    // Decode and validate continuation bytes
    if (bytes_needed == 1) {
        code_point = s[0];
    } else {
        // Initial mask for the first byte
        if (bytes_needed == 2) {
            code_point = s[0] & 0x1F;
        } else if (bytes_needed == 3) {
            code_point = s[0] & 0x0F;
        } else if (bytes_needed == 4) {
            code_point = s[0] & 0x07;
        }

        for (i = 1; i < bytes_needed; ++i) {
            // Check if continuation byte
            if ((s[i] & 0xC0) != 0x80) {
                return -1;
            }
            code_point = (code_point << 6) | (s[i] & 0x3F);
        }
    }

    // Check for overlong encoding
    // 2-byte sequence must be >= 0x80
    if (bytes_needed == 2 && code_point < 0x80) return -1;
    // 3-byte sequence must be >= 0x800
    if (bytes_needed == 3 && code_point < 0x800) return -1;
    // 4-byte sequence must be >= 0x10000
    if (bytes_needed == 4 && code_point < 0x10000) return -1;

    // Check for surrogate halves (U+D800 to U+DFFF)
    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;

    // Check for code points above U+10FFFF
    if (code_point > 0x10FFFF) return -1;

    *cp = code_point;
    return bytes_needed;
}