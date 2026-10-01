#include <stdint.h>

static inline unsigned popcount(unsigned x) {
    x ^= x >> 16;
    x *= 0x0000ffffu;
    x += x >> 8;
    x &= 0x1f1f1f1fu;
    x *= 0x01010101u;
    return x >> 24;
}