#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi,
                size_t q, long long *out) {
    if (n == 0 || q == 0) {
        return;
    }

    /* Build prefix sum array: prefix[i] = sum(a[0]...a[i-1]) */
    /* Allocate on heap for large n to avoid stack overflow */
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (prefix == NULL) {
        return; /* Allocation failure; cannot process */
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }

    /* Answer each query in O(1) */
    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t r = hi[i];
        /* Sum from index l to r inclusive is prefix[r+1] - prefix[l] */
        out[i] = prefix[r + 1] - prefix[l];
    }

    free(prefix);
}