#include <stdint.h>

int cmp_desc(const void *a, const void *b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    int64_t diff = (int64_t)*ia - (int64_t)*ib;

    if (diff > 0) return 1;
    if (diff < 0) return -1;
    return 0;
}