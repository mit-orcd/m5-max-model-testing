#include <stdint.h>

int cmp_desc(const void *a, const void *b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    int64_t x = (int64_t)*ia;
    int64_t y = (int64_t)*ib;
    return (x > y) ? 1 : ((x < y) ? -1 : 0);
}