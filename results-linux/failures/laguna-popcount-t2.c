#include <stdint.h>

static inline unsigned popcount(unsigned x) {
    x ^= x >> 16;
    x += x >> 32;
    x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
    return (x & 0x33333333) + ((x >> 2) & 0x33333333);
}