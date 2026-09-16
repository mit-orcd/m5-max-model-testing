#include <stddef.h>

void range_sums(const int *a, size_t n, const size_t *lo, const size_t *hi, size_t q, long long *out) {
    long long *pref = (long long *)0;
    if (n > 0) {
        pref = (long long *)malloc((n + 1) * sizeof(long long));
        pref[0] = 0;
        for (size_t i = 0; i < n; i++) {
            pref[i + 1] = pref[i] + a[i];
        }
    }

    for (size_t i = 0; i < q; i++) {
        out[i] = pref[hi[i] + 1] - pref[lo[i]];
    }

    if (pref) {
        free(pref);
    }
}