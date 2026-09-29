#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    size_t i, j;
    for (i = 0; i < q; i++) {
        out[i] = 0;
        for (j = lo[i]; j <= hi[i]; j++) {
            out[i] += a[j];
        }
    }
}