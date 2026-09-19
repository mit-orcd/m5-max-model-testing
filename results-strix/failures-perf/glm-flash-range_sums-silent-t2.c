#include <stddef.h>
#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums to answer range sum queries in O(1) time per query.
    // The prefix sum array S is defined such that S[k] = sum_{i=0}^{k-1} a[i].
    // This means S[0] = 0 and S[k] = a[0] + a[1] + ... + a[k-1].
    
    long long *S = (long long *)malloc((n + 1) * sizeof(long long));
    if (S == NULL) {
        // Handle memory allocation failure if necessary
        return;
    }

    S[0] = 0;
    for (size_t i = 0; i < n; i++) {
        S[i + 1] = S[i] + a[i];
    }

    for (size_t i = 0; i < q; i++) {
        out[i] = S[hi[i] + 1] - S[lo[i]];
    }

    free(S);
}