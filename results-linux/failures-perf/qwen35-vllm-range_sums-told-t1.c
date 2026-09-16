#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Allocate prefix sum array on stack or heap. 
    // Since n can be up to hundreds of thousands, we use dynamic allocation 
    // to avoid stack overflow, though in practice a static buffer or 
    // passing a pre-allocated buffer would be faster if memory management is handled externally.
    // However, per the function signature, we must compute it here.
    
    // We assume the caller ensures we have enough memory or we allocate it.
    // To strictly adhere to the signature without additional parameters for scratch space,
    // we allocate the prefix sum array.
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (prefix == NULL) {
        return; // Handle allocation failure gracefully
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + a[i];
    }

    for (size_t i = 0; i < q; ++i) {
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }

    free(prefix);
}