#include <stddef.h>
#include <stdint.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    size_t *prefix = NULL;
    
    // Allocate prefix sum array
    prefix = (size_t *)malloc(n * sizeof(size_t));
    if (prefix == NULL) return;

    // Build prefix sum array (using size_t to avoid overflow during accumulation if n is large,
    // though the output is long long, intermediate sums of indices are safe, but we need sum of values).
    // Correction: We need prefix sums of the VALUES, not indices.
    // Re-implementing with correct logic for value sums.
    
    // Free the incorrectly allocated buffer and reallocate properly
    free(prefix);
    
    // Actually, let's just allocate and compute correctly.
    // Since the values in 'a' are int, the sum can exceed int range, so we store long long in prefix.
    // However, the problem signature provides 'out' as long long, so we must compute long long.
    // We can use a temporary array for prefix sums.
    
    long long *psum = (long long *)malloc(n * sizeof(long long));
    if (psum == NULL) return;

    if (n > 0) {
        psum[0] = a[0];
        for (size_t i = 1; i < n; i++) {
            psum[i] = psum[i-1] + a[i];
        }
    }

    for (size_t i = 0; i < q; i++) {
        size_t l = lo[i];
        size_t r = hi[i];
        if (l == 0) {
            out[i] = psum[r];
        } else {
            out[i] = psum[r] - psum[l - 1];
        }
    }

    free(psum);
}