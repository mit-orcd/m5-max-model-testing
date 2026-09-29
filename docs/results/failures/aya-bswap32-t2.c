#include <stdint.h>

uint32_t bswap32(uint32_t x) {
    return __bswap_32(x);
}