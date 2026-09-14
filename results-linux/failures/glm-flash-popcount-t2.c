#include <stdalign.h>

unsigned popcount(unsigned x) {
    alignas(16) unsigned int buffer[4] = {0};
    __builtin_ia32_popcntd(&buffer[0], x);
    return buffer[0];
}