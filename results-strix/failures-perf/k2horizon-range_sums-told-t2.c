#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (q == 0) return;

    size_t *pref = (size_t *)0;
    long long *sums = (long long *)0;
    int *valid = (int *)0;

    sums = (long long *)malloc((n + 1) * sizeof(long long));
    if (sums == 0) return;
    sums[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        sums[i + 1] = sums[i] + (long long)a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        out[i] = sums[hi[i] + 1] - sums[lo[i]];
    }

    free(sums);
}