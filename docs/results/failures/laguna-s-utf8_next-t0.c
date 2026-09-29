#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char b0 = s[0];
    uint32_t codepoint;
    int len;

    // Determine the expected sequence length based on the leading byte
    if ((b0 & 0x80) == 0x00) {
        // 1-byte sequence: 0xxxxxxx
        codepoint = b0;
        len = 1;
    } else if ((b0 & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx
        codepoint = b0 & 0x1F;
        len = 2;
    } else if ((b0 & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx
        codepoint = b0 & 0x0F;
        len = 3;
    } else if ((b0 & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx
        codepoint = b0 & 0x07;
        len = 4;
    } else {
        // Invalid leading byte (continuation byte or 5/6-byte form)
        return -1;
    }

    // Check if there are enough bytes remaining
    if (n < (size_t)len) return -1;

    // Process continuation bytes
    for (int i = 1; i < len; i++) {
        unsigned char b = s[i];
        if ((b & 0xC0) != 0x80) return -1; // Not a valid continuation byte
        codepoint = (codepoint << 6) | (b & 0x3F);
    }

    // Validate the decoded code point
    if (len == 2 && codepoint < 0x80) return -1;                 // Overlong
    if (len == 3 && codepoint < 0x800) return -1;                // Overlong
    if (len == 4 && codepoint < 0x10000) return -1;              // Overlong
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return -1;   // Surrogate
    if (codepoint > 0x10FFFF) return -1;                         // Out of range

    *cp = codepoint;
    return len;
}