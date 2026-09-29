#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums to allow O(1) per query
    // prefix[i] = sum of a[0]...a[i-1], with prefix[0] = 0
    // Then sum(a[lo]...a[hi]) = prefix[hi+1] - prefix[lo]
    
    // We can compute prefix sums on the fly or precompute.
    // Given n is up to hundreds of thousands, precomputing is efficient.
    
    // Use a static or heap-allocated array for prefix sums.
    // Since we can't allocate large arrays on stack, and the function signature doesn't allow us to pass a buffer,
    // we'll use a simple approach: compute prefix sums into a dynamically allocated array.
    
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