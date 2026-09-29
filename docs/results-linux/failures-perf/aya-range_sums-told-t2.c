#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    long long sum = 0;
    for (size_t i = 0; i < q; i++) {
        sum += a[hi[i]] - a[lo[i] - 1];
        out[i] = sum;
    }
}