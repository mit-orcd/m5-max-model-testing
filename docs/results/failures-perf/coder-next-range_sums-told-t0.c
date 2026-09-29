#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Build prefix sum array
    long long *prefix = (long long *)__builtin_assume_aligned(
        _Alignof(long long), 
        _Alignof(long long) == 8 ? 
            (void *)malloc((n + 1) * sizeof(long long)) : 
            (void *)malloc((n + 1) * sizeof(long long))
    );
    
    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + a[i];
    }
    
    // Answer queries using prefix sums
    for (size_t i = 0; i < q; ++i) {
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }
    
    free(prefix);
}