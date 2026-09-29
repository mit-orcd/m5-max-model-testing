#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) {
        for (size_t i = 0; i < q; i++) {
            out[i] = 0;
        }
        return;
    }

    // Allocate prefix sum array
    // prefix[k] = sum of a[0] ... a[k-1]
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) {
        // Fallback: should not happen in valid scenarios, but handle gracefully
        for (size_t i = 0; i < q; i++) {
            out[i] = 0;
        }
        return;
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + a[i];
    }

    for (size_t i = 0; i < q; i++) {
        size_t l = lo[i];
        size_t h = hi[i];
        // Sum from l to h inclusive = prefix[h+1] - prefix[l]
        out[i] = prefix[h + 1] - prefix[l];
    }

    free(prefix);
}