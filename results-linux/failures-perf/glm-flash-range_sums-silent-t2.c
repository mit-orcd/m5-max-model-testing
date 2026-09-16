#include <stddef.h>
#include <limits.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0) return;

    long long prefix[2][n + 1];
    size_t k = 0;

    for (size_t i = 0; i < n; ++i) {
        prefix[k][i + 1] = prefix[k][i] + a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t r = hi[i];

        if (l == 0) {
            out[i] = prefix[k][r + 1];
        } else {
            out[i] = prefix[k][r + 1] - prefix[k][l];
        }
    }

    k = 1;
    for (size_t i = 0; i < n; ++i) {
        prefix[k][i + 1] = prefix[k][i] + a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t r = hi[i];

        if (l == 0) {
            out[i] = prefix[k][r + 1];
        } else {
            out[i] = prefix[k][r + 1] - prefix[k][l];
        }
    }
}