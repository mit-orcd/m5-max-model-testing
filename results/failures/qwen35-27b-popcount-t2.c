#include <stdint.h>

unsigned popcount(unsigned x) {
    x = x - ((x >> 1) & 0x77777777UL);
    x = (x & 0x55555555UL) + ((x >> 1) & 0x55555555UL);
    x = (x + (x >> 2)) & 0x33333333UL;
    x = x + (x >> 4);
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x3F;
}