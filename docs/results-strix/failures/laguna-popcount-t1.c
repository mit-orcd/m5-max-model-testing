#include <stdint.h>

static inline unsigned popcount(unsigned x) {
    x ^= x >> 16;
    x += x >> 8;
    x *= 0x01010101;
    return x >> 24;
}