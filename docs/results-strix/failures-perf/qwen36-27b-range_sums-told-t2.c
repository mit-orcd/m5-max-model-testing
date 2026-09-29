void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums
    // prefix[0] = 0, prefix[i] = a[0] + ... + a[i-1]
    // Sum from lo to hi inclusive = prefix[hi+1] - prefix[lo]
    
    // Allocate prefix sum array dynamically or use static if n is bounded, 
    // but since n can be up to hundreds of thousands, we should allocate on heap or stack.
    // To avoid malloc overhead in a performance-critical context, we could use VLA if n is known at runtime and compiler supports it, 
    // but for portability and safety with large n, let's use malloc.
    
    // However, for maximum performance and avoiding allocation, if n is within reasonable stack limits, we could use alloca or VLA.
    // Given the constraint "hundreds of thousands", a VLA on stack might be risky (stack overflow). 
    // Let's use malloc for safety, but note that in competitive programming contexts, sometimes global arrays are used.
    // Since the function signature is fixed, we'll use malloc.
    
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) return; // Should not happen in valid usage
    
    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }
    
    for (size_t i = 0; i < q; i++) {
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }
    
    free(prefix);
}