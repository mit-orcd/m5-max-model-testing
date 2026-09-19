#include <stddef.h>
#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Build prefix sum array
    // Use static or stack allocation if n is moderate, but for hundreds of thousands
    // dynamic allocation is safer to avoid stack overflow.
    // However, since we cannot use malloc in a pure function signature without leaking
    // or requiring free, and the constraints imply performance, we will compute
    // prefix sums in a way that avoids O(n) extra space if possible, 
    // OR use a large static buffer if the environment allows.
    // Given "hundreds of thousands", a static array of 1,000,000 long longs is safe for most competitive contexts.
    // To be strictly compliant with C11 and avoid static limits or heap overhead in a tight loop:
    // We will allocate on the heap. It's the standard approach for this scale.
    
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) return; // Should not happen in valid test cases

    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t h = hi[i];
        // Sum from a[l] to a[h] inclusive is prefix[h+1] - prefix[l]
        out[i] = prefix[h + 1] - prefix[l];
    }

    free(prefix);
}