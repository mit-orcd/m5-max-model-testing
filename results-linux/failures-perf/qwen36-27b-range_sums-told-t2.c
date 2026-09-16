void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums to allow O(1) per query
    // prefix[i] = sum of a[0]...a[i-1], so sum(a[l]...a[r]) = prefix[r+1] - prefix[l]
    // We can use a local array or dynamically allocate. Since n can be up to hundreds of thousands,
    // stack allocation might be risky on some systems, but typically 100k * 8 bytes = 800KB is fine.
    // However, to be safe and efficient, we'll use a VLA or static buffer if possible.
    // Given C11, VLA is optional but widely supported. Alternatively, we can allocate on heap.
    // But for performance and simplicity, let's use a VLA. If n is very large, this might overflow stack.
    // A safer approach for "hundreds of thousands" is to allocate on heap or use a static buffer.
    // Let's use alloca for stack allocation which is common in competitive programming contexts,
    // or just a VLA. Let's stick to VLA for standard C11 compliance where supported.
    // Actually, for maximum safety and performance, let's just compute prefix sums in a dynamically allocated array.
    // But the problem says no main, no tests. We need to be efficient.
    // Allocating 800KB on stack is usually fine. Let's use a VLA.
    
    long long prefix[n + 1];
    prefix[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + a[i];
    }
    
    for (size_t i = 0; i < q; ++i) {
        // sum from lo[i] to hi[i] inclusive
        // = prefix[hi[i] + 1] - prefix[lo[i]]
        out[i] = prefix[hi[i] + 1] - prefix[lo[i]];
    }
}