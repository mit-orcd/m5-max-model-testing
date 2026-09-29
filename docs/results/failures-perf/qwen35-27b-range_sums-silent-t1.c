void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    size_t i;
    for (i = 0; i < q; i++) {
        long long sum = 0;
        size_t j;
        for (j = lo[i]; j <= hi[i]; j++) {
            sum += a[j];
        }
        out[i] = sum;
    }
}