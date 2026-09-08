#include <stdbit.h>

unsigned popcount(unsigned x) {
    return __builtin_popcount(x);
}