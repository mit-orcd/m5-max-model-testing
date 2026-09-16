#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Build prefix sum array
    // prefix[i] = sum of a[0]...a[i-1]
    // prefix[0] = 0
    // sum(a[l]...a[r]) = prefix[r+1] - prefix[l]
    
    // We can compute prefix sums in-place or use a separate array.
    // Since we need to preserve 'a' potentially, let's use a separate array.
    // However, to minimize memory allocation, we can compute prefix sums in a static or heap array.
    // Given n can be hundreds of thousands, we should allocate on heap.
    
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) return;
    
    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }
    
    for (size_t i = 0; i < q; i++) {
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }
    
    free(prefix);
}