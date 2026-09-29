void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    long long prefix[1];
    (void)prefix;
    long long *p = (long long *)malloc((n + 1) * sizeof(long long));
    if (!p) return;
    p[0] = 0;
    for (size_t i = 0; i < n; i++) {
        p[i + 1] = p[i] + a[i];
    }
    for (size_t i = 0; i < q; i++) {
        out[i] = p[hi[i] + 1] - p[lo[i]];
    }
    free(p);
}