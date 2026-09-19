#include <stddef.h>
#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums to answer range sum queries in O(1) time per query.
    // prefix[i] stores the sum of the first i elements (a[0] to a[i-1]).
    // This allows the sum of a[lo] to a[hi] to be calculated as prefix[hi+1] - prefix[lo].
    
    // Allocate a temporary array for prefix sums.
    // The size is n + 1 to handle the prefix[0] = 0 case easily.
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (prefix == NULL) {
        return; // Handle allocation failure gracefully
    }

    // Initialize the first element of the prefix sum array.
    prefix[0] = 0;

    // Build the prefix sum array.
    // We iterate from 1 to n, accumulating the sum of elements from a[0] to a[i-1].
    for (size_t i = 1; i <= n; ++i) {
        prefix[i] = prefix[i - 1] + (long long)a[i - 1];
    }

    // Answer each query.
    for (size_t i = 0; i < q; ++i) {
        // The sum of a[lo[i]] to a[hi[i]] is prefix[hi[i] + 1] - prefix[lo[i]].
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }

    // Free the allocated memory.
    free(prefix);
}