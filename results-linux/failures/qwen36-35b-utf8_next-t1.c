#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t codepoint = s[0];
    int bytes_needed = 0;

    if (codepoint < 0x80) {
        // Single byte: U+0000 to U+007F
        bytes_needed = 1;
    } else if (codepoint < 0xC0) {
        // Continuation byte in leading position
        return -1;
    } else if (codepoint < 0xE0) {
        // Two bytes: U+0080 to U+07FF
        bytes_needed = 2;
    } else if (codepoint < 0xF0) {
        // Three bytes: U+0800 to U+FFFF
        bytes_needed = 3;
    } else if (codepoint < 0xF8) {
        // Four bytes: U+10000 to U+10FFFF (and obsolete 5/6 byte starts)
        bytes_needed = 4;
    } else {
        // 0xF8-0xFF are invalid or obsolete 5/6 byte starts
        return -1;
    }

    // Check if enough bytes are available
    if (n < (size_t)bytes_needed) {
        return -1;
    }

    // Decode and validate
    if (bytes_needed == 1) {
        *cp = codepoint;
        return 1;
    }

    // Helper to get next byte and check validity
    uint32_t accumulated = codepoint & ((1 << (6 - (bytes_needed - 1) * 5)) - 1); 
    // Actually, simpler logic: mask out the prefix bits for the first byte
    // 2-byte: 110xxxxx -> xxxx
    // 3-byte: 1110xxxx -> xxxx
    // 4-byte: 11110xxx -> xxx
    
    // Let's do it byte by byte for clarity and strictness
    
    uint32_t result = 0;
    int i;
    
    // First byte processing
    if (bytes_needed == 2) {
        if ((codepoint & 0xE0) != 0xC0) return -1; // Must be 110xxxxx
        result = codepoint & 0x1F;
    } else if (bytes_needed == 3) {
        if ((codepoint & 0xF0) != 0xE0) return -1; // Must be 1110xxxx
        result = codepoint & 0x0F;
    } else if (bytes_needed == 4) {
        if ((codepoint & 0xF8) != 0xF0) return -1; // Must be 11110xxx
        result = codepoint & 0x07;
    }

    // Process continuation bytes
    for (i = 1; i < bytes_needed; ++i) {
        uint8_t cont = s[i];
        if ((cont & 0xC0) != 0x80) {
            return -1; // Not a continuation byte 10xxxxxx
        }
        result = (result << 6) | (cont & 0x3F);
    }

    // Check for overlong encodings
    // Min value for 2 bytes is 0x80 (11000000 10000000)
    // Min value for 3 bytes is 0x800 (11100000 10000000 10000000)
    // Min value for 4 bytes is 0x10000 (11110000 10000000 10000000 10000000)
    
    if (bytes_needed == 2 && result < 0x80) return -1;
    if (bytes_needed == 3 && result < 0x800) return -1;
    if (bytes_needed == 4 && result < 0x10000) return -1;

    // Check for surrogates (U+D800 to U+DFFF)
    if (result >= 0xD800 && result <= 0xDFFF) return -1;

    // Check for max valid code point U+10FFFF
    if (result > 0x10FFFF) return -1;

    *cp = result;
    return bytes_needed;
}