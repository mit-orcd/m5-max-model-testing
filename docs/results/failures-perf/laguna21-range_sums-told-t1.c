#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0) return;

    long long *prefix = (long long *)malloc(n * sizeof(long long));
    if (!prefix) return;

    prefix[0] = a[0];
    for (size_t i = 1; i < n; ++i) {
        prefix[i] = prefix[i - 1] + a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t h = hi[i];
        if (l == 0) {
            out[i] = prefix[h];
        } else {
            out[i] = prefix[h] - prefix[l - 1];
        }
    }

    free(prefix);
}