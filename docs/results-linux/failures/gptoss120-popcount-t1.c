#include <stddef.h>

static inline unsigned popcount(unsigned x)
{
    /* Kernighan's algorithm: repeatedly clear the lowest set bit */
    unsigned count = 0;
    while (x) {
        x &= x - 1;
        ++count;
    }
    return count;
}