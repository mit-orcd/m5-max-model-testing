#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    long long *prefix = (long long *)calloc(n + 1, sizeof(long long));
    if (prefix == NULL) {
        return;
    }

    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t r = hi[i];
        out[i] = prefix[r + 1] - prefix[l];
    }

    free(prefix);
}