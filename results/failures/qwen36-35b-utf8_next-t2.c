#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t code_point;
    size_t bytes_needed;
    unsigned char first_byte = s[0];

    // Determine expected length and mask for code point bits
    if ((first_byte & 0x80) == 0x00) {
        // 1-byte sequence: 0xxxxxxx
        bytes_needed = 1;
        code_point = first_byte;
    } else if ((first_byte & 0xE0) == 0xC0) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        bytes_needed = 2;
        if (n < bytes_needed) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        code_point = (first_byte & 0x1F) << 6;
        code_point |= (s[1] & 0x3F);
    } else if ((first_byte & 0xF0) == 0xE0) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        bytes_needed = 3;
        if (n < bytes_needed) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        code_point = (first_byte & 0x0F) << 12;
        code_point |= (s[1] & 0x3F) << 6;
        code_point |= (s[2] & 0x3F);
    } else if ((first_byte & 0xF8) == 0xF0) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        bytes_needed = 4;
        if (n < bytes_needed) return -1;
        if ((s[1] & 0xC0) != 0x80) return -1;
        if ((s[2] & 0xC0) != 0x80) return -1;
        if ((s[3] & 0xC0) != 0x80) return -1;
        code_point = (first_byte & 0x07) << 18;
        code_point |= (s[1] & 0x3F) << 12;
        code_point |= (s[2] & 0x3F) << 6;
        code_point |= (s[3] & 0x3F);
    } else {
        // Invalid leading byte (continuation byte or reserved)
        return -1;
    }

    // Check for overlong encodings
    // 2-byte form minimum is 0x80
    if (bytes_needed == 2 && code_point < 0x80) return -1;
    // 3-byte form minimum is 0x800
    if (bytes_needed == 3 && code_point < 0x800) return -1;
    // 4-byte form minimum is 0x10000
    if (bytes_needed == 4 && code_point < 0x10000) return -1;

    // Check for UTF-16 surrogates (U+D800 - U+DFFF)
    if (code_point >= 0xD800 && code_point <= 0xDFFF) return -1;

    // Check for maximum valid code point (U+10FFFF)
    if (code_point > 0x10FFFF) return -1;

    *cp = code_point;
    return (int)bytes_needed;
}