#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t codepoint = 0;
    int bytes_needed = 0;
    size_t i = 0;

    // Determine expected length and initial mask/value based on first byte
    if ((s[0] & 0x80) == 0) {
        // 1-byte sequence: 0xxxxxxx
        bytes_needed = 1;
        codepoint = s[0];
    } else if ((s[0] & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx
        bytes_needed = 2;
        codepoint = s[0] & 0x1F;
    } else if ((s[0] & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx
        bytes_needed = 3;
        codepoint = s[0] & 0x0F;
    } else if ((s[0] & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx
        bytes_needed = 4;
        codepoint = s[0] & 0x07;
    } else {
        // Invalid start byte (continuation byte or invalid pattern)
        return -1;
    }

    // Check if we have enough bytes remaining
    if (n < (size_t)bytes_needed) {
        return -1;
    }

    // Process continuation bytes
    for (i = 1; i < (size_t)bytes_needed; ++i) {
        // Must be a continuation byte: 10xxxxxx
        if ((s[i] & 0xC0) != 0x80) {
            return -1;
        }
        codepoint = (codepoint << 6) | (s[i] & 0x3F);
    }

    // Check for overlong encoding
    // 2-byte sequence should not encode values < 0x80
    if (bytes_needed == 2 && codepoint < 0x80) return -1;
    // 3-byte sequence should not encode values < 0x800
    if (bytes_needed == 3 && codepoint < 0x800) return -1;
    // 4-byte sequence should not encode values < 0x10000
    if (bytes_needed == 4 && codepoint < 0x10000) return -1;

    // Check for UTF-16 surrogates (U+D800 to U+DFFF)
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;

    // Check for values above maximum valid Unicode code point
    if (codepoint > 0x10FFFF) return -1;

    // All checks passed
    *cp = codepoint;
    return bytes_needed;
}