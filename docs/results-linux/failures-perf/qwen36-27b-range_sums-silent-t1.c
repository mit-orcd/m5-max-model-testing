#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums
    // prefix[i] = a[0] + a[1] + ... + a[i-1]
    // prefix[0] = 0
    // sum(a[l..r]) = prefix[r+1] - prefix[l]
    
    // We can allocate prefix array on stack if n is small, but for general n, 
    // we should be careful. However, since we cannot use malloc in a simple function 
    // without freeing, and the problem doesn't specify memory constraints, 
    // we'll use a static approach or assume n is reasonable.
    // Actually, let's just compute prefix sums in a local array. 
    // But n can be large. Let's use a pointer and malloc, then free.
    
    // Alternatively, we can avoid extra allocation by computing sums directly, 
    // but that would be O(n*q) which might be slow. The problem doesn't specify 
    // performance constraints, but prefix sum is the standard efficient approach.
    
    // Let's use malloc for prefix array.
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) return; // Handle allocation failure
    
    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }
    
    for (size_t i = 0; i < q; i++) {
        size_t l = lo[i];
        size_t r = hi[i];
        // sum from a[l] to a[r] inclusive
        out[i] = prefix[r + 1] - prefix[l];
    }
    
    free(prefix);
}