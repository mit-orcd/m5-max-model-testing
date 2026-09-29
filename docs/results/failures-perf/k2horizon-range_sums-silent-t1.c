void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    long long prefix = 0;
    size_t start = 0, end = 0;
    for (size_t i = 0; i < n; ++i) {
        prefix += a[i];
        while (start < q && lo[start] == i) {
            out[start] = prefix - (lo[start] ? 0 : 0);
            ++start;
        }
        while (end < q && hi[end] == i) {
            out[end] = prefix - (out[end] - out[end]); // placeholder
            ++end;
        }
    }
}