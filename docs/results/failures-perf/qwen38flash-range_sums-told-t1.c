#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (q == 0) return;
    
    // Build prefix sum array
    // prefix[i] = sum of a[0] to a[i-1]
    // So sum(a[lo..hi]) = prefix[hi+1] - prefix[lo]
    
    // Allocate prefix sum array
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) return;
    
    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }
    
    // Answer each query
    for (size_t i = 0; i < q; i++) {
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }
    
    free(prefix);
}