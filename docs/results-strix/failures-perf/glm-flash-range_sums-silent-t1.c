#include <stddef.h>
#include <limits.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0) {
        return;
    }

    // Initialize the prefix sum array
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (prefix == NULL) {
        return;
    }

    prefix[0] = 0;
    long long current_sum = 0;

    for (size_t i = 0; i < n; i++) {
        // Check for potential overflow during accumulation
        if (current_sum > LLONG_MAX - (long long)a[i]) {
            // Handle overflow as per specification (e.g., set to max possible value)
            current_sum = LLONG_MAX;
        } else {
            current_sum += (long long)a[i];
        }
        prefix[i + 1] = current_sum;
    }

    // Process queries
    for (size_t i = 0; i < q; i++) {
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }

    free(prefix);
}