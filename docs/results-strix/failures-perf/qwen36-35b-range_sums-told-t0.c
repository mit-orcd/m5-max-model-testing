#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Build prefix sum array
    // prefix[i] = sum of a[0]...a[i-1]
    // prefix[0] = 0
    // sum(a[l]...a[r]) = prefix[r+1] - prefix[l]
    
    // We can compute prefix sums in-place or use a separate array.
    // Since we need to preserve 'a' potentially, let's use a separate array.
    // However, to minimize memory allocation overhead, we can allocate once.
    // But the function signature doesn't allow us to allocate. We must use stack or assume caller handles it?
    // Actually, we can't allocate inside without knowing if it's allowed. 
    // Let's use a VLA or dynamic allocation. VLA is C99/C11 but not always recommended.
    // Given n is up to hundreds of thousands, a VLA on stack might overflow.
    // Let's use dynamic allocation with malloc.
    
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