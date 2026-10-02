#include <stdint.h>

uint32_t bswap32(uint32_t x) {
    return ((x >> 24) & 0x000000FF) |
           ((x >> 8)  & 0x0000FF00) |
           ((x << 8)  & 0x0000FF00) |
           ((x << 24) & 0xFF000000);
}