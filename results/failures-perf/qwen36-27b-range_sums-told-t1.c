#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Build prefix sum array
    // prefix[i] = a[0] + a[1] + ... + a[i-1]
    // Sum from lo to hi inclusive = prefix[hi+1] - prefix[lo]
    
    // Allocate prefix sum array on heap or stack? 
    // n is up to hundreds of thousands, so stack allocation might be risky but usually fine for 10^5 * 8 bytes = 800KB
    // However, to be safe and efficient, we can use a local array if n is reasonable, or malloc.
    // But malloc has overhead. Let's use a VLA or static allocation? 
    // C11 VLAs are optional. Let's use malloc for safety, but that adds overhead.
    // Alternatively, we can compute prefix sums in a pre-allocated buffer passed from outside? 
    // The function signature is fixed. We must allocate internally.
    
    // For performance, let's allocate prefix sum array.
    // Since n can be up to ~10^5-10^6, malloc is acceptable.
    
    long long *prefix = malloc((n + 1) * sizeof(long long));
    if (!prefix) return; // Should not happen in competitive programming context
    
    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + a[i];
    }
    
    for (size_t i = 0; i < q; i++) {
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }
    
    free(prefix);
}