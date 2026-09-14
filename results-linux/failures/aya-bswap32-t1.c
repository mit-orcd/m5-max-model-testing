#include <stdint.h>

uint32_t bswap32(uint32_t x) {
    x = (x << 8) | (x >> 8);
    return ((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8);
}