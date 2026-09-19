void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums
    // prefix[i] = sum of a[0] ... a[i-1], so prefix[0] = 0
    // sum from lo to hi inclusive = prefix[hi+1] - prefix[lo]
    
    // Use dynamic allocation or stack? n can be hundreds of thousands, so stack might overflow.
    // But the problem says to implement the function. We can use alloca for small n, but for safety
    // and since n can be large, let's use malloc. However, the signature doesn't allow returning extra
    // data, and we need to free it. Let's use a static buffer if n is within reasonable bounds,
    // or malloc. Since this is a competitive programming style problem, malloc is fine, but we must free.
    // Actually, for performance, let's try to avoid malloc overhead if possible. But n is up to 10^5 or 10^6.
    // A static array of size 10^6 * sizeof(long long) = 8MB, which is fine on the stack? No, stack is limited.
    // Let's use malloc.
    
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) return; // Should not happen in competitive context
    
    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }
    
    for (size_t i = 0; i < q; i++) {
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }
    
    free(prefix);
}