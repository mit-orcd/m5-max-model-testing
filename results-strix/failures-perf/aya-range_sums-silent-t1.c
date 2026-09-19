void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    for (size_t i = 0; i < q; i++) {
        size_t l = lo[i];
        size_t h = hi[i];
        long long s = 0;
        for (size_t j = l; j <= h; j++) {
            s += a[j];
        }
        out[i] = s;
    }
}