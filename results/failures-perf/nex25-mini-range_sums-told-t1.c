```c
#include <stddef.h>
#include <stdlib.h>

void range_sums(const int *a, size_t n, const size_t *lo,
                const size_t *hi, size_t q, long long *out) {
    long long *prefix = malloc((n + 1)