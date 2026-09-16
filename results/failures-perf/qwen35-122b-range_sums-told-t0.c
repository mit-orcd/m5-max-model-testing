#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums to allow O(1) range sum queries.
    // prefix[i] will store the sum of a[0]...a[i-1].
    // prefix[0] = 0.
    // Sum of a[lo]...a[hi] = prefix[hi+1] - prefix[lo].
    
    // Allocate prefix array on the stack is risky for large n, so we use a pointer.
    // However, since we cannot allocate dynamically without stdlib in a pure snippet context 
    // (and malloc is allowed in C11), we will use a local array if n is small or dynamic.
    // Given n is in the hundreds of thousands, we must use dynamic allocation or a static buffer.
    // To be safe and standard-compliant for large inputs, we use malloc.
    
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) {
        // In a real scenario, handle error. Here we assume success as per competitive programming context.
        return;
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t r = hi[i];
        // Sum from l to r inclusive is prefix[r+1] - prefix[l]
        out[i] = prefix[r + 1] - prefix[l];
    }

    free(prefix);
}