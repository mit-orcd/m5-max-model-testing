#include <stddef.h>

int cmp_desc(const void *a, const void *b) {
    long long lhs = *(const long long *)a;
    long long rhs = *(const long long *)b;
    if (lhs > rhs) return -1;
    if (lhs < rhs) return 1;
    return 0;
}