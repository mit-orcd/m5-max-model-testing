void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    long long *prefix_sum = (long long *)malloc((n + 1) * sizeof(long long));
    
    prefix_sum[0] = 0;
    for (size_t i = 0; i < n; i++) {
        prefix_sum[i + 1] = prefix_sum[i]yte] + (long long)a[i];
    }
    
    for (size_t i = 0; i < q; i++) {
        out[i] = prefix_sum[hi[i] + 1] - prefix_sum[lo[i]];
    }
    
    free(prefix_sum);
}