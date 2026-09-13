#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    for (size_t i = 0; i < q; i++) {
        out[i] = 0;
        for (size_t j = lo[i]; j <= hi[i]; j++) {
            out[i] += a[j];
        }
    }
}