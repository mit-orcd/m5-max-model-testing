#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;
    uint32_t code = 0, len = 0;
    if (s[0] < 0x80) {
        *cp = s[0];
        return 1;
    }
    for (len = 1; len < 5; ++len) {
        if (s[len] < 0x80 && s[len] != 0) {
            if (len == 1) return -1;  // continuation byte in leading position
            if (len > 3 && (s[0] & 0xf8) == 0xf8) return -1;  // 5-byte form
            if (len > 4 && (s[0] & 0xf0) == 0xf0) return -1;  // 6-byte form
            if (len == 2 && (s[0] | s[1]) < 0x800) return -1;  // overlong 2-byte
            if (len == 3 && (s[0] | s[1] | s[2]) < 0x10000) return -1;  // overlong 3-byte
            if (len == 4 && (s[0] | s[1] | s[2] | s[3]) < 0x200000) return -1;  // overlong 4-byte
            if (len == 5 && (s[0] | s[1] | s[2] | s[3] | s[4]) < 0x4000000) return -1;  // overlong 5-byte
            if (len == 6 && (s[0] | s[1] | s[2] | s[3] | s[4] | s[5]) < 0x80000000) return -1;  // overlong 6-byte
            code = (code << 6) | (s[len] & 0x3f);
            if (len == 2 && code >= 0xd800 && code <= 0xdfff) return -1;  // utf16 surrogate
            if (len == 4 && code > 0x10ffff) return -1;  // out of range
            break;
        } else {
            return -1;  // bad continuation byte
        }
    }
    *cp = code;
    return len;
}