#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    size_t i;
    long long sum = 0;
    for (i = 0; i < q; i++) {
        size_t j = lo[i];
        while (j <= hi[i]) {
            sum += a[j];
            j++;
        }
        out[i] = sum;
    }
}