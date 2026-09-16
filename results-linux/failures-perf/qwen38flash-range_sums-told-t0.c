#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) {
        return;
    }

    // Build prefix sums
    // prefix[i] = sum of a[0] ... a[i-1]
    // We need prefix array of size n+1
    // To avoid extra allocation, we can compute prefix sums in-place if allowed,
    // but a is const. So we need a temporary array or compute on the fly.
    // Given n and q are large, O(n + q) is optimal.
    // We'll allocate a temporary prefix array.
    
    // Since we cannot allocate dynamically in a strict embedded context without stdlib,
    // but the problem doesn't forbid stdlib, we can use malloc.
    // However, to be safe and efficient, let's assume we can use a local VLA or malloc.
    // VLA might be too large for stack. Let's use malloc.
    
    // Actually, let's try to avoid allocation if possible by processing queries differently?
    // No, prefix sum is the standard O(1) per query approach.
    
    // Let's use malloc for the prefix array.
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) {
        // Handle allocation failure gracefully? The problem doesn't specify.
        // We'll just return or set outputs to 0.
        for (size_t i = 0; i < q; i++) {
            out[i] = 0;
        }
        return;
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + a[i];
    }

    for (size_t i = 0; i < q; i++) {
        // sum from lo[i] to hi[i] inclusive
        // = prefix[hi[i] + 1] - prefix[lo[i]]
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }

    free(prefix);
}