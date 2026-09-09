#include <stddef.h>
#include <limits.h>

int cmp_desc(const void *a, const void *b) {
    const int ia = *(const int *)a;
    const int ib = *(const int *)b2;
    
    if (ia == ib) return 0;
    if (ia < ib) return 1;
    return -1;
}