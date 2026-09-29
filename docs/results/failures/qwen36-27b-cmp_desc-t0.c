#include <stdint.h>

int cmp_desc(const void *a, const void *b) {
    const int ia = *(const int *)a;
    const int ib = *(const int *)b;

    if (ia > 0 && ib < 0) return 1;
    if (ia < 0 && ib > 0) return -1;

    return (ib - ia) > 0 ? 1 : (ib - ia) < 0 ? -1 : 0;
}