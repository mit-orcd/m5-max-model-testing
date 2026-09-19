#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0) return;
    
    long long *prefix_sums = (long long *)malloc((n + 1) * sizeof(long long));
    if (prefix_sums == NULL) return;
    
    prefix_sums[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix_sums[i + 1] = prefix_sums[i] + a[i];
    }
    
    for (size_t i = 0; i < q; ++i) {
        out[i] = prefix_sums[hi[i] + 1] - prefix_sums[lo[i]];
    }
    
    free(prefix_sums);
}