#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums
    // prefix[i] = sum of a[0]...a[i-1]
    // sum(a[l]...a[r]) = prefix[r+1] - prefix[l]
    
    // We can compute prefix sums in place if we're careful, but let's use a local approach.
    // Since we need random access to prefix sums, we should store them.
    // However, allocating memory for n elements might be expensive if n is large.
    // But given the constraints, we need O(n + q) time.
    
    // Let's compute prefix sums into a temporary array.
    // Note: We cannot allocate on stack if n is very large, but in C11 with VLA or malloc...
    // Since we're in a function, let's use malloc for the prefix array.
    
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