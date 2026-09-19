#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    size_t i, j;
    long long sum;

    for (i = 0; i < q; i++) {
        sum = 0;
        for (j = lo[i]; j <= hi[i]; j++) {
            sum += a[j];
        }
        out[i] = sum;
    }
}