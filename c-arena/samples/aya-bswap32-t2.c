#include <stdint.h>

uint32_t bswap32(uint32_t x) {
    return (x << 24) | ((x & 0xFF00) >> 8) | ((x & 0xFF) << 8) | (x >> 24);
}