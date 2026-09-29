#include <stdint.h>

static inline uint32_t bswap32(uint32_t x)
{
    return ((x >> 24) & 0x000000FFU) |
           ((x >> 8)  & 0x0000FF00U) |
           ((x << 8)  & 0x00FF0000U) |
           ((x << 24) & 0xFF000000U);
}