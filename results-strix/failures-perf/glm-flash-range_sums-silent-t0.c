#include <stddef.h>
#include <limits.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums to answer each query in O(1) time.
    // We use a long long array to prevent integer overflow during accumulation.
    long long prefix[n + 1];
    
    // Initialize the prefix sum array.
    // prefix[0] is defined as 0 to simplify the calculation for the first element.
    prefix[0] = 0;
    
    for (size_t i = 0; i < n; i++) {
        // Check for potential overflow before adding the current element.
        // If adding a[i] to prefix[i] would exceed LLONG_MAX, we clamp it.
        if (prefix[i] > LLONG_MAX - (long long)a[i]) {
            prefix[i + 1] = LLONG_MAX;
        } else {
            prefix[i + 1] = prefix[i] + (long long)a[i];
        }
    }

    // Answer each query using the prefix sum array.
    for (size_t i = 0; i < q; i++) {
        // The sum of a[lo[i]] to a[hi[i]] is prefix[hi[i] + 1] - prefix[lo[i]].
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }
}