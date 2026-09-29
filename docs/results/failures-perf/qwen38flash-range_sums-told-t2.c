#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Build prefix sums
    // prefix[i] = sum of a[0] .. a[i-1], so prefix[0] = 0
    // Then sum from lo to hi inclusive = prefix[hi + 1] - prefix[lo]
    
    // We need an array of size n+1 for prefix sums
    // Use a temporary array. Since n can be hundreds of thousands, stack allocation might be risky.
    // Let's allocate dynamically or use a large enough approach.
    // Actually, let's just compute prefix sums into a temporary buffer.
    
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) {
        // Handle error, but for competitive programming style, assume valid input
        return;
    }
    
    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }
    
    for (size_t i = 0; i < q; i++) {
        size_t l = lo[i];
        size_t r = hi[i];
        out[i] = prefix[r + 1] - prefix[l];
    }
    
    free(prefix);
}