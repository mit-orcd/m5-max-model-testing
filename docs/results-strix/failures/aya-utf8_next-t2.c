#include <stdint.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    uint32_t cp_ = 0xFF >> ((s[0] >> 3) & 0x1F);  // assume 1-byte form
    if (cp_ & 0x80) {  // if first bit is 1
        if ((s[0] & 0xDF) == 0xDF) return -1;  // invalid 2nd or 3rd byte
        if (n < 2) return -1;  // truncated
        cp_ = (cp_ & 0x7F) << 6 | (s[1] & 0x3F);
        if (cp_ < 0x80) return -1;  // overlong 2-byte form
        if (cp_ >= 0x10000) return -1;  // out of range
        if (n < 3) return -1;  // truncated
        if ((s[1] & 0xC0) != 0x80) return -1;  // bad 2nd byte
        if ((s[2] & 0xC0) != 0x80) return -1;  // bad 3rd byte
        cp_ |= (s[2] & 0x3F) << 12;
        if (cp_ >= 0x110000) return -1;  // out of range
        if (n < 4) return -1;  // truncated
        if ((s[2] & 0x80) != 0x80) return -1;  // bad 4th byte
        cp_ |= (s[3] & 0x3F) << 18;
        if (cp_ >= 0x200000) return -1;  // out of range
        if (n < 5) return -1;  // truncated
        if ((s[3] & 0xC0) != 0x80) return -1;  // bad 5th byte
        cp_ |= (s[4] & 0x3F) << 24;
        if (cp_ >= 0x4000000) return -1;  // out of range
    }
    *cp = cp_;
    return (cp_ < 0x10000) ? 1 : ((cp_ < 0x200000) ? 2 : ((cp_ < 0x4000000) ? 3 : 4));
}