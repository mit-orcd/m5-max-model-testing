```c
#include <stdlib.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    if (n == 0 || q == 0) {
        return;
    }
    long long *pref = (long long *)malloc((n + 1) * sizeof(long long));
    if (!pref) {
        return;
    }
    pref[0] = 0;
    for (size_t i = 0; i < n; ++i) {
        pref[i + 1] = pref[i] + a[i];
    }
    for (size_t i = 0; i < q; ++i) {
        size_t l