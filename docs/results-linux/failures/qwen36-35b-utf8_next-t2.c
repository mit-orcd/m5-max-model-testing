#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) {
        return -1;
    }

    uint32_t code_point = s[0];
    int bytes_needed = 0;

    if ((code_point & 0x80) == 0) {
        // 1-byte sequence: 0xxxxxxx
        bytes_needed = 0;
    } else if ((code_point & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        bytes_needed = 1;
    } else if ((code_point & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        bytes_needed = 2;
    } else if ((code_point & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        bytes_needed = 3;
    } else {
        // Invalid leading byte (10xxxxxx or 11111xxx+)
        return -1;
    }

    // Check if we have enough bytes remaining
    if (n <= (size_t)bytes_needed) {
        return -1;
    }

    // Decode and check continuation bytes
    if (bytes_needed >= 1) {
        if ((s[1] & 0xC0) != 0x80) return -1;
        code_point = ((code_point & 0x1F) << 6) | (s[1] & 0x3F);
    }
    if (bytes_needed >= 2) {
        if ((s[2] & 0xC0) != 0x80) return -1;
        code_point = (code_point << 6) | (s[2] & 0x3F);
    }
    if (bytes_needed >= 3) {
        if ((s[3] & 0xC0) != 0x80) return -1;
        code_point = (code_point << 6) | (s[3] & 0x3F);
    }

    // Check for overlong encodings
    if (bytes_needed == 1 && code_point < 0x80) {
        return -1;
    }
    if (bytes_needed == 2 && code_point < 0x800) {
        return -1;
    }
    if (bytes_needed == 3 && code_point < 0x10000) {
        return -1;
    }
    if (bytes_needed == 4 && code_point < 0x200000) {
        return -1;
    }

    // Reject UTF-16 surrogates
    if (code_point >= 0xD800 && code_point <= 0xDFFF) {
        return -1;
    }

    // Reject values above U+10FFFF
    if (code_point > 0x10FFFF) {
        return -1;
    }

    *cp = code_point;
    return bytes_needed + 1;
}