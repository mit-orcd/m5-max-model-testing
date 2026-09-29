#include <stddef.h>
#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) {
        for (size_t i = 0; i < q; i++) {
            out[i] = 0;
        }
        return;
    }

    // Build prefix sums: prefix[i] = sum of a[0]..a[i-1], prefix[0] = 0
    // We need prefix of size n+1
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) {
        // Fallback: compute each query directly (O(n*q))
        for (size_t i = 0; i < q; i++) {
            long long s = 0;
            for (size_t j = lo[i]; j <= hi[i]; j++) {
                s += a[j];
            }
            out[i] = s;
        }
        return;
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + a[i];
    }

    for (size_t i = 0; i < q; i++) {
        // sum a[lo[i]..hi[i]] = prefix[hi[i]+1] - prefix[lo[i]]
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }

    free(prefix);
}