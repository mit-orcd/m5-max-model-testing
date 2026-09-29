#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) {
        return;
    }

    // Build prefix sum array.
    // Use long long to prevent overflow during accumulation.
    // We allocate a temporary array for prefix sums on the stack if n is small,
    // but since n can be up to hundreds of thousands, we use dynamic allocation
    // or a static buffer if constraints allow. However, standard C11 doesn't allow
    // VLA for very large sizes reliably in all contexts without stack overflow risks.
    // To be safe and efficient, we compute prefix sums in-place or use a separate array.
    // Given the constraints, we will allocate a prefix sum array.
    
    // Note: Since we cannot return a pointer from this void function easily without
    // modifying the signature, and we cannot allocate inside the function and return
    // the pointer to the caller (as out is the output array for queries), we must
    // compute the prefix sums in a way that doesn't require returning a pointer.
    // We will allocate a prefix sum array on the heap.
    
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (prefix == NULL) {
        return; // Allocation failed
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        size_t l = lo[i];
        size_t h = hi[i];
        // Sum from l to h inclusive is prefix[h+1] - prefix[l]
        out[i] = prefix[h + 1] - prefix[l];
    }

    free(prefix);
}