#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) return;

    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }

    free(prefix);
}