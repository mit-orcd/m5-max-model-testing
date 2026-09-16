#include <stdint.h>

unsigned popcount(unsigned x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    return (((x + (x >> 4)) * 0x0F0F0F0F) >> 24) & 0xFF;
}