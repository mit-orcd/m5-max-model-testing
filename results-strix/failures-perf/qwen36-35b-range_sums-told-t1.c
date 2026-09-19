#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Build prefix sum array
    // prefix[i] = sum of a[0]...a[i-1]
    // prefix[0] = 0
    // sum(a[l]...a[r]) = prefix[r+1] - prefix[l]
    
    // We can compute prefix sums on the fly or precompute.
    // Given n is up to hundreds of thousands, precomputing is better.
    
    // However, we need to be careful about memory. Let's use a local array for prefix sums.
    // But n can be large, so we might want to avoid large stack allocation.
    // Actually, since we're in a function, we can use malloc or just compute prefix sums in a single pass if we process queries smartly.
    // But queries are arbitrary, so prefix sum array is the standard approach.
    
    // Let's use a static or heap-allocated prefix sum array. Since we can't guarantee the caller manages memory,
    // and we don't know if we can use malloc, let's think again.
    
    // Actually, in competitive programming and such scenarios, using malloc is acceptable.
    // But let's check: the problem says "implement in C11", so malloc is available.
    
    // Alternative: process queries offline by sorting, but that's O(q log q) which might be slower.
    // Prefix sum is O(n + q), which is optimal.
    
    // Let's use a heap-allocated prefix sum array.
    
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