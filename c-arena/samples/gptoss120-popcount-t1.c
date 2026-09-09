#include <stddef.h>

static inline unsigned popcount(unsigned x)
{
    unsigned count = 0;
    while (x) {
        x &= x - 1;   // clear the least significant set bit
        ++count;
    }
    return count;
}