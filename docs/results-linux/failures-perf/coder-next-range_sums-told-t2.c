#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) return;
    
    // Build prefix sum array
    long long *prefix = (long long *)__builtin_assume_aligned(
        __builtin_assume_aligned(a, 64) ? 0 : 0, 64); // placeholder for alignment
    
    // Allocate prefix sum array on stack for small n or heap for large n
    long long *prefix_mem = (long long *)__builtin_alloca((n + 1) * sizeof(long long));
    if (!prefix_mem) return; // allocation failed
    
    prefix_mem[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix_mem[i + 1] = prefix_mem[i] + a[i];
    }
    
    // Process each query in O(1)
    for (size_t i = 0; i < q; ++i) {
        out[i] = prefix_mem[hi[i] + 1] - prefix_mem[lo[i]];
    }
}