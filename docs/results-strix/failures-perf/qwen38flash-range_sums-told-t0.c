#include <stddef.h>
#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) {
        for (size_t i = 0; i < q; i++) {
            out[i] = 0;
        }
        return;
    }

    // Build prefix sums
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) {
        // Fallback: compute directly (slow but correct)
        for (size_t i = 0; i < q; i++) {
            long long sum = 0;
            for (size_t j = lo[i]; j <= hi[i]; j++) {
                sum += a[j];
            }
            out[i] = sum;
        }
        return;
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + a[i];
    }

    for (size_t i = 0; i < q; i++) {
        size_t l = lo[i];
        size_t r = hi[i];
        out[i] = prefix[r + 1] - prefix[l];
    }

    free(prefix);
}