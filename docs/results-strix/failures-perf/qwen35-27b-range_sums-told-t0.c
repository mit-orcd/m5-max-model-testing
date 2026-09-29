#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) {
        return;
    }

    // Precompute prefix sums
    // prefix[i] = sum(a[0]...a[i-1]), prefix[0] = 0
    // We use a local array for prefix sums to avoid allocation overhead if n is small,
    // but for n in hundreds of thousands, stack allocation might be risky.
    // However, the problem implies a single call context. 
    // To be safe and efficient for large n, we can use a static buffer or dynamic allocation.
    // Given the constraints and typical competitive programming environments, 
    // a static buffer of reasonable size or dynamic allocation is needed.
    // Since n can be large, let's use dynamic allocation for the prefix array.
    
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (prefix == NULL) {
        return; // Handle allocation failure gracefully
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }

    // Answer queries
    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t h = hi[i];
        // Sum from a[l] to a[h] inclusive is prefix[h+1] - prefix[l]
        out[i] = prefix[h + 1] - prefix[l];
    }

    free(prefix);
}