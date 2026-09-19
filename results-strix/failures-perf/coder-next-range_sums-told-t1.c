#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) return;
    
    // Build prefix sum array
    long long *prefix = (long long *)__builtin_assume_aligned(a, 64);
    prefix = (long long *)__builtin_assume_aligned(prefix, 64);
    
    // We need a separate prefix array since input a is int*
    long long *pref = (long long *)__builtin_assume_aligned(out, 64);
    
    // Use out temporarily for prefix sum storage (safe since we'll overwrite it)
    pref[0] = a[0];
    for (size_t i = 1; i < n; ++i) {
        pref[i] = pref[i-1] + a[i];
    }
    
    // Answer queries using prefix sums
    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t r = hi[i];
        out[i] = pref[r] - (l > 0 ? pref[l-1] : 0);
    }
}