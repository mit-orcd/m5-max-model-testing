void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0 || a == NULL || lo == NULL || hi == NULL || out == NULL) {
        return;
    }

    // Build prefix sum array
    // prefix[i] = sum of a[0]...a[i-1]
    // prefix[0] = 0
    // sum(a[lo]...a[hi]) = prefix[hi+1] - prefix[lo]
    
    long long *prefix = (long long *)malloc((n + 1) * sizeof(long long));
    if (prefix == NULL) {
        return;
    }

    prefix[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + (long long)a[i];
    }

    for (size_t i = 0; i < q; i++) {
        size_t l = lo[i];
        size_t h = hi[i];
        out[i] = prefix[h + 1] - prefix[l];
    }

    free(prefix);
}