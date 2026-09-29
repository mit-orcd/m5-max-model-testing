#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n > 4) n = 4;  // at most 4 bytes
    for (size_t i = 0; i < n; i++) {
        if ((s[i] & 0xc0) != 0x80) {  // not a continuation byte
            if (i == 0) return -1;  // continuation byte in leading position
            if (s[i] < 0x20) return -1;  // control or ASCII FFFE/FFFE
            if (s[i] <= 0x7f) {
                *cp = s[i];
                return 1;
            }
            if (s[i] <= 0xf7) {  // 2-byte: 0x00-0xff
                if (i == n - 1) return -1;  // truncated
                if ((s[i + 1] & 0xc0) != 0x80) return -1;
                *cp = (s[i] & 0xf) << 6 | s[i + 1] & 0x3f;
                return 2;
            }
            if (s[i] <= 0xef) {  // 3-byte: 0x80-0x7ff
                if (i == n - 2) return -1;  // truncated
                if ((s[i + 1] & 0xc0) != 0x80 || (s[i + 2] & 0xc0) != 0x80) return -1;
                *cp = (s[i] & 0xf) << 12 | (s[i + 1] & 0x3f) << 6 | s[i + 2] & 0x3f;
                return 3;
            }
            if (s[i] <= 0xf4) {  // 4-byte: 0x1000-0x10ffff
                if (i == n - 3) return -1;  // truncated
                if ((s[i + 1] & 0xc0) != 0x80 || (s[i + 2] & 0xc0) != 0x80 || (s[i + 3] & 0xc0) != 0x80) return -1;
                *cp = (s[i] & 0xf) << 18 | (s[i + 1] & 0x3f) << 12 | (s[i + 2] & 0x3f) << 6 | s[i + 3] & 0x3f;
                return 4;
            }
        }
    }
    return -1;  // truncated
}