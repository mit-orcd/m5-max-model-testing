void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0) return;
    
    // Build prefix sum array in-place using a local array to avoid allocation overhead if possible,
    // but since we can't allocate large stack arrays safely, we'll use a dynamic approach or reuse.
    // However, the signature doesn't allow us to allocate. We must compute sums efficiently.
    // Since we can't modify 'a' and don't have extra space in the signature, we need to precompute
    // prefix sums. But we can't allocate. Wait, the problem says "Implement in C11", and only provides
    // the function signature. We can allocate memory inside the function.
    
    // For O(1) per query, we need prefix sums. Let's allocate a prefix sum array.
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