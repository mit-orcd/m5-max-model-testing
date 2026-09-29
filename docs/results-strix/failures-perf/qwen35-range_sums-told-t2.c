#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Build prefix sum array
    // Use a stack-allocated or temporary buffer if n is small, but for n in hundreds of thousands,
    // we must allocate dynamically or assume a large enough static buffer.
    // Since we cannot return the buffer or use global state in a library function easily,
    // and the problem implies a single call, we allocate a prefix sum array.
    // Note: In a real production environment with strict memory constraints, a static buffer or
    // pre-allocated array would be preferred. Here we use malloc for safety with large n.
    
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (prefix == NULL) {
        return; // Handle allocation failure gracefully
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t h = hi[i];
        out[i] = prefix[h + 1] - prefix[l];
    }

    free(prefix);
}