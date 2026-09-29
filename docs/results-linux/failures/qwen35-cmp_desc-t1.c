#include <limits.h>

int cmp_desc(const void *a, const void *b) {
    const int va = *(const int *)a;
    const int vb = *(const int *)b;

    if (va > 0 && vb < 0) return 1;
    if (va < 0 && vb > 0) return -1;

    if (va > vb) return 1;
    if (va < vb) return -1;
    return 0;
}