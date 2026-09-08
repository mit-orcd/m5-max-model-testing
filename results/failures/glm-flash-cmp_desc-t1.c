#include <stddef.h>

int cmp_desc(const void *a, const void *b) {
    const int ia = *(const int *)a;
    const int ib = *(const int *)b2;
    if (ia == ib) return 0;
    return (ia > ib) ? -1 : 1;
}