#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Allocate and compute prefix sums
    // prefix[i] will store sum(a[0]...a[i-1]), prefix[0] = 0
    // We need size n+1 to handle the range [lo, hi] inclusive as prefix[hi+1] - prefix[lo]
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (prefix == NULL) {
        // In a real scenario, handle error appropriately.
        // For this snippet, we assume allocation succeeds or caller handles it.
        return;
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }

    // Answer queries in O(1) each
    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t r = hi[i];
        // Sum from l to r inclusive is prefix[r+1] - prefix[l]
        out[i] = prefix[r + 1] - prefix[l];
    }

    free(prefix);
}