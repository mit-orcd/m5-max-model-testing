void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    for (size_t i = 0; i < q; i++) {
        long long sum = 0;
        for (im = lo[i]; im <= hi[i]; im++) {
            sum += a[im];
        }
        out[i] = sum;
    }
}