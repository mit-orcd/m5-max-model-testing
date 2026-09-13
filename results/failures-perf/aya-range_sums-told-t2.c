#include <stddef.h>
#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    size_t i;
    for (i = 0; i < q; i++) {
        out[i] = 0;
        while (lo[i] < hi[i]) {
            out[i] += a[lo[i]];
            lo[i]++;
        }
        out[i] += a[hi[i]];
    }
}