#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    size_t i;
    for (i = 0; i < q; i++) {
        out[i] = 0;
        size_t j = lo[i];
        for (size_t k = hi[i]; k >= j; k--) {
            out[i] += a[k];
        }
    }
}