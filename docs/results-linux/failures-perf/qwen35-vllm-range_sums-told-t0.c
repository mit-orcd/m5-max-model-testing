#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    long long *prefix = (long long *)a;
    if (prefix != (long long *)a) {
        // This cast is unsafe if a is not aligned or if we need to preserve a.
        // However, the problem signature implies we cannot allocate extra memory easily
        // or the caller expects us to use the provided buffer if possible, but we can't
        // overwrite 'a' if it's const.
        // Since we cannot allocate O(n) memory inside the function without knowing the allocator,
        // and the signature doesn't provide a buffer for prefix sums, we must use the O(n) per query
        // approach? No, that would be O(n*q) which is too slow for 10^5 * 10^5.
        // We must allocate a prefix sum array.
        // But wait, the function signature is fixed. We can use malloc.
    }
    
    // Allocate prefix sum array
    long long *p = (long long *)malloc((n + 1) * sizeof(long long));
    if (!p) return;

    p[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        p[i + 1] = p[i] + a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        out[i] = p[hi[i] + 1] - p[lo[i]];
    }

    free(p);
}