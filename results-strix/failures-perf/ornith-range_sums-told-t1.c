void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    size_t *pref = (size_t *) malloc((n + 1) * sizeof(size_t));
    pref[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        pref[i + 1] = pref[i] + (size_t)a[i];
    }
    for (size_t i = 0; i < q; ++i) {
        long long sum = (long long)pref[hi[i] + 1] - (long long)pref[lo[i]];
        out[i] = sum;
    }
    free(pref);
}