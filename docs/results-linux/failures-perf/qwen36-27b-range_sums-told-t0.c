void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    // Precompute prefix sums
    // prefix[i] = a[0] + a[1] + ... + a[i-1]
    // sum(a[l..r]) = prefix[r+1] - prefix[l]
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (!prefix) return; // Handle allocation failure gracefully

    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }

    for (size_t i = 0; i < q; i++) {
        size_t l = lo[i];
        size_t r = hi[i];
        out[i] = prefix[r + 1] - prefix[l];
    }

    free(prefix);
}