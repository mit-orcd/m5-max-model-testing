#include <limits.h>

int cmp_desc(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;

    if (x > y) return 1;
    if (x < y) return -1;
    return 0;
}