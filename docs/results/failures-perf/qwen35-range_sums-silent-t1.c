void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    for (size_t i = 0; i < q; ++i) {
        long long sum = 0;
        size_t start = lo[i];
        size_t end = hi[i];
        for (size_t j = start; j <= end; ++j) {
            sum += a[j];
        }
        out[i] = sum;
    }
}