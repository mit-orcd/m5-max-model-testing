#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums
    // We use a local array on the stack if n is small, but for hundreds of thousands
    // we should allocate dynamically or use a static buffer if reentrancy isn't required.
    // Since we cannot allocate on the heap inside this function without malloc/free (which might be slow),
    // and the problem implies a single call or performance critical, we assume the caller
    // might provide a buffer or we use a static buffer if we assume single-threaded usage.
    // However, the standard safe approach for a function signature like this without a context struct
    // is to allocate. But malloc for 100k longs is fast enough.
    // To be strictly compliant and efficient without external dependencies beyond C11:
    
    long long *prefix = NULL;
    if (n > 0) {
        prefix = (long long *)malloc(n * sizeof(long long));
        if (!prefix) return; // Allocation failure, though unlikely for n in 100k
        
        prefix[0] = a[0];
        for (size_t i = 1; i < n; i++) {
            prefix[i] = prefix[i - 1] + a[i];
        }
    }

    for (size_t i = 0; i < q; i++) {
        size_t l = lo[i];
        size_t h = hi[i];
        long long sum;
        if (l == 0) {
            sum = prefix[h];
        } else {
            sum = prefix[h] - prefix[l - 1];
        }
        out[i] = sum;
    }

    if (n > 0) {
        free(prefix);
    }
}