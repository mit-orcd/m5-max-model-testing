#include <stdint.h>

uint32_t bswap32(uint32_t x) {
    return (x & 0x00000001) << 3 | (x & 0x00000010) << 1 |
           (x & 0x00001000) >> 1 | (x & 0x00010000) >> 3;
}