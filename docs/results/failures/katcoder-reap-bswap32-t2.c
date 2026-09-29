#include <stdint.h>

uint32_t bswap32(uint32_t x) {
    return (x & 0x00000000UL) |
           ((x & 0x000000FFUL) << 24) |
           ((x & 0x0000FF00UL) << 16) |
           ((x & 0x00FF0000UL) << 8) |
           ((x & 0xFF000000UL) >> 24);
}